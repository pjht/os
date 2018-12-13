#ifndef PS2_INT_H
#define PS2_INT_H

#define PS2_PORT_INT(port) (port==1?33:44)

char ps2_read_data();
void ps2_write_data(char byte);
void ps2_write_data_to_device(int port,char data);
char ps2_send_cmd_to_device(int port,char cmd);
char ps2_send_cmd_w_data_to_device(int port,char cmd,char data);


#endif
