// CPPExample.cpp : main project file.

#include "stdafx.h"

using namespace System;
using namespace ::Kinova::API::Jaco;
using namespace ::Kinova::API::Jaco::Configurations;
using namespace ::Kinova::DLL::Data;
using namespace ::Kinova::DLL::Data::Jaco::Config;
using namespace ::Kinova::DLL::Data::Jaco;
using namespace ::Kinova::DLL::SafeGate;

int main(array<System::String ^> ^args)
{
    Console::WriteLine(L"C + + Example");

	//Your password provided by Kinova to access the API
    String^ m_APIpass = "MyValidPassword";

	//Your JACO
	CJacoArm^ jaco = gcnew CJacoArm(Crypto::GetInstance()->Encrypt(m_APIpass));

	//If JACO is connected
	if (jaco->JacoIsReady())
	{
		Console::WriteLine(L"JACO is up and ready ...");
		Console::WriteLine(L"");
		Console::WriteLine(L"");

		//We load client data from JACO
		CClientConfigurations^ ClientConfig = jaco->ConfigurationsManager->GetClientConfigurations();

		//We modify the client as we want
		ClientConfig->Organization = "MyOrganization";
		ClientConfig->ClientNo = "1234";
		ClientConfig->ClientName = "MyName";
		ClientConfig->Laterality = CJacoStructures::ArmLaterality::RightHandedness;

		//We save the data to JACO
		jaco->ConfigurationsManager->SetClientConfigurations(ClientConfig);

		ClientConfig = jaco->ConfigurationsManager->GetClientConfigurations();

		//Display the data
		Console::WriteLine(L"*** INFORMATION ABOUT CLIENT ***");
		Console::WriteLine(L"");
		Console::WriteLine(L"    Organization = " + ClientConfig->Organization);
		Console::WriteLine(L"     Client name = " + ClientConfig->ClientName);
		Console::WriteLine(L"       Client no = " + ClientConfig->ClientNo);
		Console::WriteLine(L"       Serial no = " + ClientConfig->SerialNo);
		Console::Write(L" JACO laterality = ");
		Console::WriteLine(ClientConfig->Laterality);
		Console::WriteLine(L"");
		Console::WriteLine(L"");
		
		
		Console::WriteLine(L"End of example");
		Console::ReadLine();
	}
	//JACO is not connected
	else
	{
		Console::WriteLine(L"JACO is offline");
		Console::WriteLine(L"End of example");
		Console::ReadLine();
	}
	
    return 0;
}
