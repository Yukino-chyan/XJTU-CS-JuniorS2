	IMPORT  |Image$$RO$$Base|

	IMPORT  |Image$$RO$$Limit|

	IMPORT  |Image$$RW$$Base|  

	IMPORT  |Image$$RW$$Limit|   

	IMPORT  |Image$$ZI$$Base| 

	IMPORT  |Image$$ZI$$Limit|  


	AREA MyCode, CODE, READONLY
	
    ENTRY     
                          
    MOV     SP, #0x400              
    LDR     R0, =0x11111111         
    LDR     R1, =0x22222222
	STMFD   SP!, {R0, R1}               
    MOV     R2, #0x0               
    CMP     R0, R1                  
    BNE     SkipNext               
    MOV     R2, #0xFF              

SkipNext
	
    LDMFD   SP!, {R3, R4}             
    B       .                       
    END