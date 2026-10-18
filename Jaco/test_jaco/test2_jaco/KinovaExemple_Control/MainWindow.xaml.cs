using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using Kinova.DLL.Data.Jaco;
using Kinova.API.Jaco;
using Kinova.DLL.SafeGate;

namespace KinovaExample_Control
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        //The thread that will send button functionnality
        Thread m_WorkThread;

        //Delegate method for the thread
        private delegate void DoWorkDelegate();
        ManualResetEvent mre = new ManualResetEvent(false);

        //The password provided by Kinova
        private const string m_APIpass = "MyValidPassword";

        //Your JACO
        private CJacoArm jaco;

        //Constructor of the window
        public MainWindow()
        {
            //Window xaml component init
            InitializeComponent();

            try
            {
                //Initialize JACO Object with an encrypted valid password
                jaco = new CJacoArm(Crypto.GetInstance().Encrypt(m_APIpass));
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("Cannot connect to jaco");
            }

            //Check if JACO is connected
            if(jaco.JacoIsReady())
            {
                ((Label)m_Grid.Children[12]).Foreground = Brushes.Green;
                ((Label)m_Grid.Children[12]).Content = "JACO is online";
            }

            //Start the thread that will send joystick data to JACO
            m_WorkThread = new Thread(new ThreadStart(DoWork));
            m_WorkThread.Priority = ThreadPriority.BelowNormal;
            m_WorkThread.IsBackground = true;
            m_WorkThread.Start();
            mre.Set();
        }

        //Closing the window EVENT
        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            m_WorkThread.Abort();
            jaco.ControlManager.StopControlAPI();
        }

        //Thread's work
        [STAThread]
        private void DoWork()
        {

            System.Windows.Threading.DispatcherOperation stdo = null;
            DoWorkDelegate updateJoystickDelegate = new DoWorkDelegate(UpdateJoystick);

            mre.WaitOne();

            //Take control of JACO
            jaco.ControlManager.StartControlAPI();


            while (m_WorkThread != null && m_WorkThread.IsAlive)
            {
                //Execute the UpdateJoystick method
                stdo = this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, updateJoystickDelegate);
                Thread.Sleep(20);
            }
            if (stdo != null)
            {
                stdo.Dispatcher.InvokeShutdown();
            }
        }
        private void UpdateJoystick()
        {
            //Declare and initialize a CJoystickValue that will emulate a GeneralJoystick
            CJoystickValue value = new CJoystickValue();

            // X UP
            if (((Button)m_Grid.Children[4]).IsPressed)
            {
                value.InclineLR = -1f;
            }
            
            // Y UP
            if (((Button)m_Grid.Children[5]).IsPressed)
            {
                value.InclineFB = -1f;
            }
            

            // Z UP
            if (((Button)m_Grid.Children[6]).IsPressed)
            {
                value.Rotate = 1f;
            }
            

            // X DOWN
            if (((Button)m_Grid.Children[7]).IsPressed)
            {
                value.InclineLR = 1f;
            }
            

            // Y DOWN
            if (((Button)m_Grid.Children[8]).IsPressed)
            {
                value.InclineFB = 1f;
            }
            

            // Z DOWN
            if (((Button)m_Grid.Children[9]).IsPressed)
            {
                value.Rotate = -1f;
            }

            // Go To Position
            if (((Button)m_Grid.Children[11]).IsPressed)
            {
                value.ButtonValue[7] = 1;
            }

            //We update JACO with the new data.
            jaco.ControlManager.SendJoystickFunctionality(value);
        }

        //If we click the Record button
        private void button1_Click(object sender, RoutedEventArgs e)
        {
            CJoystickValue value = new CJoystickValue();

            //We send a 1 for a pressed
            value.ButtonValue[6] = 1;

            jaco.ControlManager.SendJoystickFunctionality(value);

            //We send a 0 for a release (Press + Release = CLICK)
            value.ButtonValue[6] = 0;

            jaco.ControlManager.SendJoystickFunctionality(value);
        }
    }
}
