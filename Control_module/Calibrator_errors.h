#ifndef CALIBRATOR_ERRORS_H_
#define CALIBRATOR_ERRORS_H_

#define NO_ERROR									0		//no error
#define ERROR_USER_INPUT					1		//user forgot '\n' or '\r' at the end, or command was written correctly, but parameter not ("VOLT a22.0")
#define ERROR_UNKNOWN_COMMAND			2		//unknown command ("VOLT:HACKUNIVERSE" or "jyvvgcvmjjk")
#define ERROR_COMMUNICATION				3		//error in writing data into register (internal problem, not user error)
#define ERROR_WRONG_MODULE				4		//UART line connected to wrong module (internal problem, not user error)
#define ERROR_VOLT_NOT_SELECTED		5		//user tries to control voltage module, but the module is not selected
#define ERROR_CURR_NOT_SELECTED		6		//user tries to control current module, but the module is not selected
#define ERROR_VOLT_RANGE					7		//specified voltage is out of selected range
#define ERROR_CURR_RANGE					8		//specified current is out of selected range
#define ERROR_FREQ_RANGE					9		//specified frequency is out of range
#define ERROR_NONEXISTENT_RANGE		10	//specified range does not exist

#endif
