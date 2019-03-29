#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "bluetooth/bluetooth.h"
#include "bluetooth/hci.h"
#include "bluetooth/hci_lib.h"

#include "dump.h"

#define EVENT_NUM 77
static char *event_str[EVENT_NUM + 1] = {
	"Unknown",
	"Inquiry Complete",
	"Inquiry Result",
	"Connect Complete",
	"Connect Request",
	"Disconn Complete",
	"Auth Complete",
	"Remote Name Req Complete",
	"Encrypt Change",
	"Change Connection Link Key Complete",
	"Master Link Key Complete",
	"Read Remote Supported Features",
	"Read Remote Ver Info Complete",
	"QoS Setup Complete",
	"Command Complete",
	"Command Status",
	"Hardware Error",
	"Flush Occurred",
	"Role Change",
	"Number of Completed Packets",
	"Mode Change",
	"Return Link Keys",
	"PIN Code Request",
	"Link Key Request",
	"Link Key Notification",
	"Loopback Command",
	"Data Buffer Overflow",
	"Max Slots Change",
	"Read Clock Offset Complete",
	"Connection Packet Type Changed",
	"QoS Violation",
	"Page Scan Mode Change",
	"Page Scan Repetition Mode Change",
	"Flow Specification Complete",
	"Inquiry Result with RSSI",
	"Read Remote Extended Features",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Synchronous Connect Complete",
	"Synchronous Connect Changed",
	"Sniff Subrate",
	"Extended Inquiry Result",
	"Encryption Key Refresh Complete",
	"IO Capability Request",
	"IO Capability Response",
	"User Confirmation Request",
	"User Passkey Request",
	"Remote OOB Data Request",
	"Simple Pairing Complete",
	"Unknown",
	"Link Supervision Timeout Change",
	"Enhanced Flush Complete",
	"Unknown",
	"User Passkey Notification",
	"Keypress Notification",
	"Remote Host Supported Features Notification",
	"LE Meta Event",
	"Unknown",
	"Physical Link Complete",
	"Channel Selected",
	"Disconnection Physical Link Complete",
	"Physical Link Loss Early Warning",
	"Physical Link Recovery",
	"Logical Link Complete",
	"Disconnection Logical Link Complete",
	"Flow Spec Modify Complete",
	"Number Of Completed Data Blocks",
	"AMP Start Test",
	"AMP Test End",
	"AMP Receiver Report",
	"Short Range Mode Change Complete",
	"AMP Status Change",
};


static void
dump_hci_event(uint8_t *buf, int len) {
	hci_event_hdr *hci_e_hdr = (hci_event_hdr *)buf;
	evt_le_meta_event *meta = (evt_le_meta_event *) (hci_e_hdr + HCI_EVENT_HDR_SIZE);
	uint8_t *payload = (uint8_t *)&meta[EVT_LE_META_EVENT_SIZE];

	len -= 1 + HCI_EVENT_HDR_SIZE + EVT_LE_META_EVENT_SIZE;
	if (hci_e_hdr->evt <= EVENT_NUM) {
		printf("HCI Event: %s (0x%2.2x) L:%d	",
					event_str[hci_e_hdr->evt], 
					hci_e_hdr->evt, hci_e_hdr->plen);
	} else
		printf("HCI dump: unexpected event: %d\n", hci_e_hdr->evt);

	do_dump_hex(payload, len);
}

void
do_dump_hex(uint8_t *buf, int len) {
	int i;
	for(i=0; i<len; i++)
		printf("%02x ", buf[i] & 0xff);
	printf("\n");
}

void
dump_hci_response(uint8_t *buf, int len) {
	uint8_t type = *(uint8_t *)buf;
	int i;

	switch (type) {
	case HCI_EVENT_PKT:
		printf("Event	");
		dump_hci_event(&buf[1], len-1);
		break;
	case HCI_COMMAND_PKT:
	case HCI_ACLDATA_PKT:
	case HCI_SCODATA_PKT:
	case HCI_VENDOR_PKT:
	default:
		printf("pkg typ %2d: ");
		do_dump_hex(buf, len);
	}
}
