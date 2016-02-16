/*
 I will write something important here at some point..
 MMN - Mathias Mikkel Neerup manee12@student.sdu.dk
 */

#include <sstream>
#include <iostream>

#include "AutoQuad.h"
#include "Session.h"

#define ST_IDLE            0
#define ST_RESET_RECV      1
#define ST_REQ_ADDR        2
#define ST_GOT_UNIQ_ADDR   3
#define ST_STREAM    4




class RegisterNode_node : public AutoQuad {

private:
	int state;
	MessageCreator messageCreator;
	int txcount = 10;

	//Make a nice plot :>
	float phase = 0;
	float value_long = 0;
	float value_lat = 0;
public:
	void recv_reset_msg() {
		ROS_WARN("Reset msg received");
		state = ST_REQ_ADDR;
	}

	void recv_ok_addr_msg(int id) {
		messageCreator.mySession.updateSession(id);

		std::cout << "Tid: " << messageCreator.mySession.tid << std::endl;
		std::cout << "Ttype: " << messageCreator.mySession.ttype << std::endl;
		std::cout << "Logic communication channel: " << messageCreator.mySession.llc << std::endl;
		std::cout << "Source id: " << messageCreator.mySession.source_id << std::endl;
		state = ST_GOT_UNIQ_ADDR;
	}

	//void recv_

	RegisterNode_node(int argc, char** argv): AutoQuad(argc, argv){
		state = ST_IDLE;
	}

	void onTimer(const ros::TimerEvent& event){
		switch(state){
		case ST_IDLE:
			break;
		case ST_REQ_ADDR:
		{
			ROS_WARN("Tx: Request addr");
			canMSG canMessage = messageCreator.Create_ReqAddr(CAN_TYPE_SENSOR, CAN_SENSORS_GPS_LAT);
			pub_recv.publish(canMessage);
			state = ST_IDLE;
		}
		break;
		case ST_GOT_UNIQ_ADDR:
			//state = ST_STREAM;
			break;

		case ST_STREAM:
			if( (txcount++) != 20 ){
				break;
			}
			txcount = 0;
			ROS_WARN("Tx: Starting stream");
			// 55.13, 11.44 = Næstved
			value_long = 0.93*sin(phase); // Vejle
			value_lat = 0.21*cos(phase); // Vejle
			phase+=0.261799;


			canMSG canMessage = messageCreator.Create_Stream(55.22 + value_lat, 10.23 + value_long);
			pub_recv.publish(canMessage);
			//	state = ST_IDLE;
			//}
			break;
		}
	}
	void spin(){
		ros::spin();
	}

	void recv_telem_rate_msg(int id){
		// Update the current session
		messageCreator.mySession.updateSession(id);

		ROS_WARN("TELEM_RATE");
		canMSG canMessage = messageCreator.Create_SendACK();
		pub_recv.publish(canMessage);
		state = ST_STREAM;
	}

	void recv_telem_value_msg(int id){
		// Update the current session
		messageCreator.mySession.updateSession(id);

		ROS_WARN("TELEM_VALUE");
		canMSG canMessage = messageCreator.Create_SendACK();
		pub_recv.publish(canMessage);
	}
};
int main(int argc, char **argv){
	AutoQuad* autoquad = new RegisterNode_node(argc, argv);


	autoquad->spin();
	return 0;
}



