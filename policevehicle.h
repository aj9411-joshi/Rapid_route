#pragma once
#include "EmergencyVehicle.h"
#include<iostream>
using namespace std;

class Policevehicle:public EmergencyVehicle{
    public:
    Policevehicle(string id,string location,int nodeId):EmergencyVehicle(id,location,nodeId){

    }
    string gettype()const override{
        return "Police";
    }
    void respondToemergency() override{
        cout<<"Policevehicle :"<<getvehicle_id()<<" location :"<<getcurrentlocation()<<endl;
    }


};
