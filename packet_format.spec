UTP custom protocol Packet

HEADER (2 bytes)
* SEQ 				(14 bits)
* TYPE				(2 bits)

INPUT LIST PAYLOAD (1000 bytes)
* INITIAL SEQ			(20 bits)
* INPUT LIST SIZE[MAX_SIZE=25]	(6 bits)

MOUSE COMMAND (40 bits)
* TYPE 				(2 bits)
* VAL1[X]			(16 bits)
* VAL2[Y] 			(16 bits)
* EV1[MOUSE_BUTTON] 		(3 bits)
* EV2[MOUSE_EVENT]		(2 bits)

SCROLL COMMAND (50 bits)
* TYPE				(2 bits)
* VAL1[X]			(16 bits)
* VAL2[Y]			(16 bits)
* EV1[OFFSET]			(16 bits)

KEY COMMAND 	(36 bits)
* TYPE				(2 bits)
* VAL1[KEY]			(16 bits)
* VAL2[SCANCODE]		(16 bits)
* EV1[KEY_EVENT]		(2 bits)

VIDEO HEADER 	(6 bytes)
* ACK				          (16 bits)
* FRAME				        (16 bits)
* INTRA_FRAME_SEQ 		(16 bits)

VIDEO PAYLOAD	(1400 bytes)