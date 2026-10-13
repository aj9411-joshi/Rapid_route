#pragma once
#include "EmergencyVehicle.h"
#include<iostream>
using namespace std;

class Firetruck:public EmergencyVehicle{
    public:
    Firetruck(string id,string location,int nodeId):EmergencyVehicle(id,location,nodeId){

    }
    string gettype()const override{
        return "Firetruck";
    }
    void respondToemergency() override{
        cout<<"Firetruck :"<<getvehicle_id()<<" location :"<<getcurrentlocation()<<endl;
    }


};
