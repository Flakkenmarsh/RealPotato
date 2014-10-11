#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

#include "tictoc13_m.h"

class Txc13 : public cSimpleModule
{
    private:
        long numSent;
        long numReceived;
        cLongHistogram hopCountStats;
        cOutVector hopCountVector;
        simsignal_t arrivalSignal;

    protected:
        virtual TicTocMsg13 *generateMessage();
        virtual void forwardMessage(TicTocMsg13 *msg);
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        void updateDisplay();
        void finish();
};

Define_Module(Txc13);

void Txc13::initialize()
{
    numSent = 0;
    numReceived = 0;
    arrivalSignal = registerSignal("arrival");
    // Module 0 sends the first message
    if (getIndex()==0)
    {
        // Boot the process scheduling the initial message as a self-message.
        TicTocMsg13 *msg = generateMessage();
        scheduleAt(0.0, msg);
    }
}

void Txc13::handleMessage(cMessage *msg)
{
    TicTocMsg13 *ttmsg = check_and_cast<TicTocMsg13 *>(msg);

    if (ttmsg->getDestination()==getIndex())
    {
        // Message arrived.
        int hopcount = ttmsg->getHopCount();
        emit(arrivalSignal, hopcount);
        EV << "Message " << ttmsg << " arrived after " << ttmsg->getHopCount() << " hops.\n";
        bubble("ARRIVED, starting new one!");
        delete ttmsg;

        // Generate another one.
        EV << "Generating another message: ";
        TicTocMsg13 *newmsg = generateMessage();
        EV << newmsg << endl;
        forwardMessage(newmsg);
    }
    else
    {
        // We need to forward the message.
        forwardMessage(ttmsg);
    }
}

TicTocMsg13 *Txc13::generateMessage()
{
    // Produce source and destination addresses.
    int src = getIndex();   // our module index
    int n = size();      // module vector size
    int dest = intuniform(0,n-2);
    if (dest>=src) dest++;

    char msgname[20];
    sprintf(msgname, "tic-%d-to-%d", src, dest);

    // Create message object and set source and destination field.
    TicTocMsg13 *msg = new TicTocMsg13(msgname);
    msg->setSource(src);
    msg->setDestination(dest);
    return msg;
}

void Txc13::forwardMessage(TicTocMsg13 *msg)
{
    // Increment hop count.
    msg->setHopCount(msg->getHopCount()+1);

    // Same routing as before: random gate.
    int n = gateSize("gate");
    int k = intuniform(0,n-1);

    EV << "Forwarding message " << msg << " on gate[" << k << "]\n";
    send(msg, "gate$o", k);
}

void Txc13::updateDisplay()
{
    char buf[40];
    sprintf(buf, "rcvd: %ld sent: %ld", numReceived, numSent);
    getDisplayString().setTagArg("t",0,buf);
}

void Txc13::finish()
{
    // This function is called by OMNeT++ at the end of the simulation.
    EV << "Sent: " << numSent << endl;
    EV << "Received: " << numReceived << endl;
    EV << "Hop count, min: " << hopCountStats.getMin() << endl;
    EV << "Hop count, max: " << hopCountStats.getMax() << endl;
    EV << "Hop count, mean: " << hopCountStats.getMean() << endl;
    EV << "Hop count, stddev: " << hopCountStats.getStddev() << endl;
    recordScalar("#sent", numSent);
    recordScalar("#received", numReceived);
    hopCountStats.recordAs("hop count");
}
