#pragma once
#include "EmergencyVehicle.h"
#include<iostream>
using namespace std;

class Ambulance:public EmergencyVehicle{
    public:
    Ambulance(string id,string location,int nodeId):EmergencyVehicle(id,location,nodeId){

    }
    string gettype()const override{
        return "Ambulance";
    }
    void respondToemergency() override{
        cout<<"ambulance :"<<getvehicle_id()<<" location :"<<getcurrentlocation()<<endl;
    }


};
