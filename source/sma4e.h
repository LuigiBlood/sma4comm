#ifndef _SMA4E_H_
#define _SMA4E_H_

/*
	e-Reader GBA Communication Menu
*/

#define EREADER_COMM_JPN	0xCCD0	//e-Reader GBA Communication Menu (JPN)
#define EREADER_COMM_USA	0xCCC0	//e-Reader GBA Communication Menu (USA)
#define EREADER_COMM_ENG	0xCC40	//e-Reader GBA Communication Menu (ENG) [Unreleased]
#define EREADER_COMM_FRA	0xCC60	//e-Reader GBA Communication Menu (FRA) [Unreleased]
#define EREADER_COMM_GER	0xCC50	//e-Reader GBA Communication Menu (GER) [Unreleased]
#define EREADER_COMM_ESP	0xCC80	//e-Reader GBA Communication Menu (ESP) [Unreleased]
#define EREADER_COMM_ITA	0xCC70	//e-Reader GBA Communication Menu (ITA) [Unreleased]

/*
	Super Mario Advance 4 e-Reader Link Commands
*/

#define SMA4E_COMM_CONNECT	0xFEFE	//Connect
#define SMA4E_COMM_RECONNECT	0xF5F5	//Reconnect

#define SMA4E_COMM_GAMEID_J	"AX4J"	//Japanese Game ID
#define SMA4E_COMM_GAMEID_E	"AX4E"	//International Game ID

#define SMA4E_COMM_ID_JPN	0xFBFB	//Send ID (JPN)
#define SMA4E_COMM_ID_USA	0xFBFB	//Send ID (USA)
#define SMA4E_COMM_ID_ENG	0xEAEA	//Send ID (ENG)
#define SMA4E_COMM_ID_FRA	0xE9E9	//Send ID (FRA)
#define SMA4E_COMM_ID_GER	0xE8E8	//Send ID (GER)
#define SMA4E_COMM_ID_ESP	0xE7E7	//Send ID (ESP)
#define SMA4E_COMM_ID_ITA	0xE6E6	//Send ID (ITA)

#define SMA4E_COMM_REQ_LVL	0xEEEE	//Request Level Card
#define SMA4E_COMM_REQ_PWR	0xEDED	//Request Power Up Card
#define SMA4E_COMM_REQ_DMO	0xECEC	//Request Demo Card

#define SMA4E_COMM_LAKITU_GONE	0xF3F3	//Lakitu is not here
#define SMA4E_COMM_LAKITU_MOVE	0xF2F2	//Lakitu is moving to slave (takes 0x12B frames maximum)
#define SMA4E_COMM_LAKITU_HERE	0xF1F1	//Lakitu is done moving

#define SMA4E_COMM_WAIT_LVL	0xFAFA	//Waiting for Level Card
#define SMA4E_COMM_WAIT_PWR	0xF0F0	//Waiting for Power Up Card
#define SMA4E_COMM_WAIT_DMO	0xEFEF	//Waiting for Demo Card

#define SMA4E_COMM_CANCEL	0xF7F7	//Cancelled Scan
#define SMA4E_COMM_WRONG	0xF8F8	//Wrong kind of Card is scanned
#define SMA4E_COMM_SCANNED	0xF9F9	//Right kind of Card is scanned

#define SMA4E_COMM_RDY_DATA	0xFEFE	//Ready to receive data
#define SMA4E_COMM_SEND_DATA	0xFDFD	//Sending Data

#define SMA4E_COMM_SENT		0xFCFC	//Data Sent

#define SMA4E_COMM_RECV_GOOD	0xF5F5	//Good Data Received
#define SMA4E_COMM_RECV_BAD	0xF4F4	//Bad Data Received

#define SMA4E_COMM_REPEATPLEASE 0xFFFF	//Only used in hardware, seems to succeed if you just resend the previous data (except before transmission has begun).

#endif
