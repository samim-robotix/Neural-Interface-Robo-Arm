using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Kinova.API.Jaco;
using Kinova.API.Jaco.Configurations;
using Kinova.API.Jaco.Control;
using Kinova.DLL.Data.Jaco.Control;
using Kinova.DLL.Data.Jaco.Config;
using Kinova.DLL.Data.Jaco;
using System.Threading;
using Kinova.DLL.Data.Util;
using Kinova.DLL.Data.Jaco.Diagnostic;
using Kinova.DLL.ReportBuilder;
using Kinova.DLL.SafeGate;
//using Kinova.DLL.TcpConnector;
using Kinova.DLL.TestData;
using Kinova.DLL.Tools;
using Kinova.DLL.USBManager;

namespace JacoArmPlanning
{
    class Program
    {
        static void Main(string[] args)
        {
            String myValidPassword = "C6H12O6h2so4";

            System.Console.WriteLine("SG");
            try
            {

                CJacoArm jArm = new CJacoArm(Crypto.GetInstance().Encrypt(myValidPassword));

                System.Console.WriteLine("API Version= " + jArm.GetAPIVersion());


                CPointsTrajectory JPointTrajectory = new CPointsTrajectory();
                if (jArm.JacoIsReady())
                {
                    jArm.ControlManager.StartControlAPI();

                    CClientConfigurations config = new CClientConfigurations();
                    config = jArm.ConfigurationsManager.GetClientConfigurations();

                    System.Console.WriteLine("Name : " + config.ClientName);
                    System.Console.WriteLine("Max linear speed: " + config.MaxLinearSpeed);
                    System.Console.WriteLine("Organization:" + config.Organization);

                    config.ClientName = "Anu";
                    config.MaxAngularSpeed = 0.1f;
                    config.Organization = "sV";

                    jArm.ConfigurationsManager.SetClientConfigurations(config);
                    config = jArm.ConfigurationsManager.GetClientConfigurations();

                    System.Console.WriteLine("Name : " + config.ClientName);
                    System.Console.WriteLine("Max linear speed:" + config.MaxLinearSpeed);
                    System.Console.WriteLine("Organization:" + config.Organization);

                    jArm.ControlManager.EraseTrajectories();

                    jArm.ControlManager.StartControlAPI();
                    CTrajectoryInfo point1 = new CTrajectoryInfo();
                    System.Console.ReadLine();

                    point1.UserPosition.PositionType = CJacoStructures.PositionType.AngularPosition;

                    point1.UserPosition.AnglesJoints.Angle[CVectorAngle.JOINT_1] = 5.317f;
                    point1.UserPosition.AnglesJoints.Angle[CVectorAngle.JOINT_2] = 176.867f;
                    point1.UserPosition.AnglesJoints.Angle[CVectorAngle.JOINT_3] = 74.093f;
                    point1.UserPosition.AnglesJoints.Angle[CVectorAngle.JOINT_4] = 226.984f;
                    point1.UserPosition.AnglesJoints.Angle[CVectorAngle.JOINT_5] = 145.217f;
                    point1.UserPosition.AnglesJoints.Angle[CVectorAngle.JOINT_6] = 60.337f;
                    point1.UserPosition.FingerPosition[0] = 0f;
                    point1.UserPosition.FingerPosition[1] = 0f;
                    point1.UserPosition.FingerPosition[2] = 0f;

                    JPointTrajectory.Add(point1);
                    Thread.Sleep(1000);
                    jArm.ControlManager.SendBasicTrajectory(JPointTrajectory);


                }
                else
                {
                    System.Console.WriteLine("Jaco is Not Ready!!");
                }
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("EXCEPTION");
                System.Console.WriteLine(ex.Message);
            }
        }
    }
}