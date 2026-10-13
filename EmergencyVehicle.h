#pragma once
#include<iostream>
#include<string>
using namespace std;

class EmergencyVehicle{
    protected:
    string vehicle_id;
    string currentlocation;
    int currentNodeId;
    public:
    EmergencyVehicle(string id,string location,int nodeId){
        vehicle_id=id;
        currentlocation=location;
        currentNodeId=nodeId;

    }
    virtual ~EmergencyVehicle() {

    }
    string getvehicle_id()const{
        return vehicle_id;
    }

    string getcurrentlocation()const{
        return currentlocation;
    }

    int getcurrentNodeId()const{
        return currentNodeId;
    }

    void setcurrentNodeId(int nodeId){
        currentNodeId=nodeId;
    }

    void updatelocation(string newlocation){
        currentlocation=newlocation;
        cout<<"["<<vehicle_id<<"] arrived at intermediate point :"<<currentlocation<<endl;

    }

    void finishtrip(string destination){
        currentlocation=destination;
        cout<<"["<<vehicle_id<<"] has arrived to the emergency destination: "<<currentlocation<<endl;
        
    }

    virtual string gettype()const=0;

    virtual void respondToemergency()=0;


};
