// sunshine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\easyloggingpp\easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main()
{
	LOG(INFO) << "logging is working";
    return 0;
}

