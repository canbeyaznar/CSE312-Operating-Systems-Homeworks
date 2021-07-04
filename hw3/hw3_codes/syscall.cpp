/* SPIM S20 MIPS simulator.
   Execute SPIM syscalls, both in simulator and bare mode.
   Execute MIPS syscalls in bare mode, when running on MIPS systems.
   Copyright (c) 1990-2010, James R. Larus.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of the James R. Larus nor the names of its contributors may be
   used to endorse or promote products derived from this software without specific
   prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _WIN32
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef _WIN32
#include <io.h>
#endif

#include "spim.h"
#include "string-stream.h"
#include "inst.h"
#include "reg.h"
#include "mem.h"
#include "sym-tbl.h"
#include "syscall.h"
#include "spim-utils.h"

#include <iostream>
using namespace std;

#ifdef _WIN32
/* Windows has an handler that is invoked when an invalid argument is passed to a system
   call. https://msdn.microsoft.com/en-us/library/a9yf33zb(v=vs.110).aspx

   All good, except that the handler tries to invoke Watson and then kill spim with an exception.

   Override the handler to just report an error.
*/

#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>

void myInvalidParameterHandler(const wchar_t *expression,
                               const wchar_t *function,
                               const wchar_t *file,
                               unsigned int line,
                               uintptr_t pReserved)
{
  if (function != NULL)
  {
    run_error("Bad parameter to system call: %s\n", function);
  }
  else
  {
    run_error("Bad parameter to system call\n");
  }
}

static _invalid_parameter_handler oldHandler;

void windowsParameterHandlingControl(int flag)
{
  static _invalid_parameter_handler oldHandler;
  static _invalid_parameter_handler newHandler = myInvalidParameterHandler;

  if (flag == 0)
  {
    oldHandler = _set_invalid_parameter_handler(newHandler);
    _CrtSetReportMode(_CRT_ASSERT, 0); // Disable the message box for assertions.
  }
  else
  {
    newHandler = _set_invalid_parameter_handler(oldHandler);
    _CrtSetReportMode(_CRT_ASSERT, 1); // Enable the message box for assertions.
  }
}
#endif

string processStates[15] = {
    "Ready", "Ready", "Ready", "Ready", "Ready",
    "Ready", "Ready", "Ready", "Ready", "Ready",
    "Ready", "Ready", "Ready", "Ready", "Ready"};

string RunningProcessNames[15];

char** ProcessNames = NULL;
int numberOfProcessName = 0;

int register_backup[15];

// This part will be read from kernel
int processID_address = 0;
int PC_ADDRESS = 0;
int endPC_address = 0;
int stackpointer_address = 0;
int parentProcess_address = 0;

int InterruptHandlerPC = 0;
int EXIT_PC = 0;
int currentProcess = 0;
int numberOfProcesses = 0;
int numberOfTerminatedProcesses = 0;
int numberOfProcesses_const = 0;

bool isTerminated = false;
bool waitpid_signal = false;

int numberOfGeneratedNumber = 0;
int randomnumbers[3];


void printProcessTable()
{
  int i = 0;
  printf("\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n");
  while (i < numberOfProcesses)
  {

    if (processStates[i] != "Finished")
    {
      printf("\n");
      printf("Process Name : %s\n", RunningProcessNames[i].c_str());
      printf("Process ID : %d\n", read_mem_word(processID_address + 4 * i));
      printf("Process state : %s\n", processStates[i].c_str());
      printf("Current PC : %d\n", read_mem_word(PC_ADDRESS + 4 * i));
      printf("End PC : %d\n", read_mem_word(endPC_address + 4 * i));
      printf("Stack Pointer Address : %d\n", read_mem_word(stackpointer_address + 4 * i));
      printf("Parent Process : %d\n", read_mem_word(parentProcess_address + 4 * i));
      printf("\n");
    }

    i++;
  }
  printf("\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n");
}

/*You implement your handler here*/
void SPIM_timerHandler()
{
  // Implement your handler..
  // if waitpid is working the interrupt will not occur
  if (waitpid_signal == false)
  {
    try
    {
      
      printf("\nINTERRUPT OCCURED!\n");
      // if the all processes are finished
      // assign PC to kernel's exit label and free all the allocated space
      if (numberOfTerminatedProcesses >= numberOfProcesses - 1)
      {
        set_mem_word(stackpointer_address + 4 * currentProcess, R[29]);
        printProcessTable();
        printf("All processes finished\n");
    
        int i;
        if(ProcessNames != NULL)
        {
          for(i=0; i<15; i++)
          {
            if(ProcessNames[i] != NULL)
              free(ProcessNames[i]);
          }
          free(ProcessNames);
        }
        
        waitpid_signal = true; // prevent interruption
        PC = EXIT_PC;          // go to the kernel's exit label
        return ;
      }

      // update the informations of the current process
      set_mem_word(PC_ADDRESS + currentProcess * 4, PC);
      set_mem_word(stackpointer_address + 4 * currentProcess, R[29]);

      // this assignment will help to kernel
      // kernel will back up the values of previously running registers
      R[26] = currentProcess;
      R[22] = register_backup[currentProcess];

      // this assigment helps us to know which program is running or ready
      if (processStates[currentProcess] != "Finished" && currentProcess != 0)
        processStates[currentProcess] = "Ready";


      // Round robin scheduling...
      int i = currentProcess + 1;
      if (i >= numberOfProcesses)
        i = 1;

      // find the ready process and run this process in assembly
      while (i != currentProcess)
      {
        if (processStates[i] != "Finished")
        {
          currentProcess = i;
          processStates[currentProcess] = "Running";
          break;
        }

        i++;

        if (i >= numberOfProcesses)
          i = 1;
      }
      if (processStates[i] != "Finished")
      {
        currentProcess = i;
        processStates[currentProcess] = "Running";
      }

      // this assignment will help to kernel
      // Kernel now knows the address of the array
      // where the registers of the process will run.
      // Thus, the kernel will assign all register values 
      // at the address of this value, and everything will be updated
      // for the new program to run. In addition, the PC value is assigned 
      // to a register so that the program can continue where it left off.
      R[23] = register_backup[currentProcess];
      R[27] = read_mem_word(PC_ADDRESS + 4 * currentProcess);

      printProcessTable();
      waitpid_signal = true;
      PC = InterruptHandlerPC - 4; // go to the kernel's interrupt_handler label

      //throw logic_error( "NotImplementedException\n" );
    }
    catch (exception &e)
    {
      cerr << endl
           << "Caught: " << e.what() << endl;
    };
  }
}
/* Decides which syscall to execute or simulate.  Returns zero upon
   exit syscall and non-zero to continue execution. */
int do_syscall()
{
#ifdef _WIN32
  windowsParameterHandlingControl(0);
#endif
/*   Syscalls for the source-language version of SPIM.  These are easier to
     use than the real syscall and are portable to non-MIPS operating
     systems. 
*/

  switch (R[REG_V0])
  {
  case PRINT_INT_SYSCALL:
    write_output(console_out, "%d", R[REG_A0]);
    break;

  case PRINT_FLOAT_SYSCALL:
  {
    float val = FPR_S(REG_FA0);

    write_output(console_out, "%.8f", val);
    break;
  }

  case PRINT_DOUBLE_SYSCALL:
    write_output(console_out, "%.18g", FPR[REG_FA0 / 2]);
    break;

  case PRINT_STRING_SYSCALL:
    write_output(console_out, "%s", mem_reference(R[REG_A0]));
    break;

  case READ_INT_SYSCALL:
  {
    static char str[256];

    read_input(str, 256);
    R[REG_RES] = atol(str);
    break;
  }

  case READ_FLOAT_SYSCALL:
  {
    static char str[256];

    read_input(str, 256);
    FPR_S(REG_FRES) = (float)atof(str);
    break;
  }

  case READ_DOUBLE_SYSCALL:
  {
    static char str[256];

    read_input(str, 256);
    FPR[REG_FRES] = atof(str);
    break;
  }

  case READ_STRING_SYSCALL:
  {
    read_input((char *)mem_reference(R[REG_A0]), R[REG_A1]);
    data_modified = true;
    break;
  }

  case SBRK_SYSCALL:
  {
    mem_addr x = data_top;
    expand_data(R[REG_A0]);
    R[REG_RES] = x;
    data_modified = true;
    break;
  }

  case PRINT_CHARACTER_SYSCALL:
    write_output(console_out, "%c", R[REG_A0]);
    break;

  case READ_CHARACTER_SYSCALL:
  {
    static char str[2];

    read_input(str, 2);
    if (*str == '\0')
      *str = '\n'; /* makes xspim = spim */
    R[REG_RES] = (long)str[0];
    break;
  }

  case EXIT_SYSCALL:
    spim_return_value = 0;
    return (0);

  case EXIT2_SYSCALL:
    spim_return_value = R[REG_A0]; /* value passed to spim's exit() call */
    return (0);

  case OPEN_SYSCALL:
  {
#ifdef _WIN32
    R[REG_RES] = _open((char *)mem_reference(R[REG_A0]), R[REG_A1], R[REG_A2]);
#else
    R[REG_RES] = open((char *)mem_reference(R[REG_A0]), R[REG_A1], R[REG_A2]);
#endif
    break;
  }

  case READ_SYSCALL:
  {
    /* Test if address is valid */
    (void)mem_reference(R[REG_A1] + R[REG_A2] - 1);
#ifdef _WIN32
    R[REG_RES] = _read(R[REG_A0], mem_reference(R[REG_A1]), R[REG_A2]);
#else
    R[REG_RES] = read(R[REG_A0], mem_reference(R[REG_A1]), R[REG_A2]);
#endif
    data_modified = true;
    break;
  }

  case WRITE_SYSCALL:
  {
    /* Test if address is valid */
    (void)mem_reference(R[REG_A1] + R[REG_A2] - 1);
#ifdef _WIN32
    R[REG_RES] = _write(R[REG_A0], mem_reference(R[REG_A1]), R[REG_A2]);
#else
    R[REG_RES] = write(R[REG_A0], mem_reference(R[REG_A1]), R[REG_A2]);
#endif
    break;
  }

  case CLOSE_SYSCALL:
  {
#ifdef _WIN32
    R[REG_RES] = _close(R[REG_A0]);
#else
    R[REG_RES] = close(R[REG_A0]);
#endif
    break;
  }

  case FORK:
  {

    break;
  }

  case EXECVE:
  {

    char *filename = ((char *)mem_reference(R[14]));
    //set process name
    RunningProcessNames[numberOfProcesses] = filename;

    if (strcmp(filename, "init") == 0)
    {
      //printf("\nINIT Process -> \n");
      //printf("init PC -> %d\n", PC);
      //printf("init current_text_pc -> %d\n", current_text_pc());

      processStates[0] = "Running";
      R[15] = PC;
      R[24] = current_text_pc();

      set_mem_word(parentProcess_address, -1);
    }

    else
    {
      // start pc
      R[15] = current_text_pc();
      read_assembly_file(filename);

      // end pc
      R[24] = current_text_pc();

      // set parent process
      set_mem_word(parentProcess_address + numberOfProcesses * 4,
                   read_mem_word(processID_address + (numberOfProcesses - 1) * 4));
    }

    // set register address 
    register_backup[numberOfProcesses] = R[REG_A0];

    // set stackpointer address
    set_mem_word(stackpointer_address + numberOfProcesses * 4, R[29]);

    numberOfProcesses++;
    break;
  }

  case WAITPID:
  {
    if (waitpid_signal == false)
      waitpid_signal = true;
    else
      waitpid_signal = false;
    break;
  }

  // this execve will run random process
  // it will take the process index from $a0
  // this execve is for SPIMOS_GTU_ 2 AND 3
  case EXECVE_2:
  {
    int index;
    
    if(numberOfGeneratedNumber == 0)
      index = R[REG_A1];
    else
      index = randomnumbers[numberOfGeneratedNumber-1];

    // start pc
    R[15] = current_text_pc();

    RunningProcessNames[numberOfProcesses] = ProcessNames[index];
    read_assembly_file(ProcessNames[index]);

    // end pc
    R[24] = current_text_pc();

    set_mem_word(parentProcess_address + numberOfProcesses * 4,
                 read_mem_word(processID_address + (numberOfProcesses - 1) * 4));

    register_backup[numberOfProcesses] = R[REG_A0];

    set_mem_word(stackpointer_address + numberOfProcesses * 4, R[29]);

    numberOfProcesses++;
    break;
  }
/*
  this syscall is used by processes to run in kernel.
  When these processes are finished,
  _TERMINATE_PROCESS syscall is called.
  and the program is terminated.
*/
  case _TERMINATE_PROCESS:
  {
    isTerminated = true;
    waitpid_signal = false;
    numberOfTerminatedProcesses++;
    processStates[currentProcess] = "Finished";

    // if the all processes are finished...
    // print the process table
    // and free all allocated parameters
    if (numberOfTerminatedProcesses >= numberOfProcesses - 1)
    {
      set_mem_word(stackpointer_address + 4 * currentProcess, R[29]);
      printProcessTable();
      printf("All processes finished\n");
   
      int i;
      if(ProcessNames != NULL)
      {
        for(i=0; i<15; i++)
        {
          if(ProcessNames[i] != NULL)
            free(ProcessNames[i]);
        }
        free(ProcessNames);
      }
      
      waitpid_signal = true; // prevent interruption
      PC = EXIT_PC;          // go to the kernel's exit label
      break;
    }

    SPIM_timerHandler();

    break;
  }

  // this syscall will create random number and it will assign it to a0 register
  // NOTE : THIS SYSCALL WILL NOT GENERATE NUMBER THAT IS BIGGER THAN R[REG_A0]'S VALUE...
  case _RANDOM_NUMBER_GENERATOR:
  {
    
    int maxBound = R[REG_A0];
    int randomNumber = rand() % maxBound;

    R[REG_A1] = randomNumber;

    break;
  }

  // this syscall is for SPIMOS_GTU_3.s 
  // this syscall will generate unique random number
  // NOTE : THIS SYSCALL WILL NOT GENERATE NUMBER THAT IS BIGGER THAN R[REG_A0]'S VALUE...
  case _RANDOM_NUMBER_GENERATOR_2:
  {
    int maxBound = R[REG_A0];
    if(numberOfGeneratedNumber == 0)
    {
      
      int randomNumber = rand() % maxBound;
      randomnumbers[numberOfGeneratedNumber] = randomNumber;
      numberOfGeneratedNumber++;
    }
    else
    {
      int randomNumber;
      int control = 0;
      int i=0;
      while(control == 0)
      {
        randomNumber = rand() % maxBound;
        for(i=0; i<numberOfGeneratedNumber; i++)
        {
          //if the random number is generated before get new random number
          if(randomnumbers[i] == randomNumber)
            break;
          else
            control++; 
        }
        if(control == numberOfGeneratedNumber)
        {
          randomnumbers[numberOfGeneratedNumber] = randomNumber;
          numberOfGeneratedNumber++;
        }
        else
          control =0;
        
      }
    }
    break;
  }
  
  // This section is used to access
  // the values of the arrays in the kernel.
  case SET_PROCESSID_ADR:
  {
    processID_address = R[REG_A0];
    break;
  }

  case SET_ENDPC_ADR:
  {
    endPC_address = R[REG_A0];
    break;
  }

  case SET_SP_ADR:
  {
    stackpointer_address = R[REG_A0];
    break;
  }

  case SET_PARENTPROCESS_ADR:
  {
    parentProcess_address = R[REG_A0];
    break;
  }

  case SET_PC_ADDRESS:
  {
    PC_ADDRESS = R[REG_A0];
    break;
  }

/*
  this syscall is the same as init syscall in my 2nd assignment. 
  Arrays that need to be defined are defined.
  And assigns the address of the interrupt_handler 
  in the kernel to a parameter.Thus, in the spimtimer_Handler () 
  function, you can handle the interrupt by 
  going to the interrupt_handler label.
*/
  case SET_INTERRUPT_HANDLER_PC:
  {
    if (InterruptHandlerPC == 0)
    {
      srand(time(NULL));
      
      int i;
      ProcessNames = (char**) malloc(sizeof(char*)*15);
      for(i=0; i<15; i++)     
        ProcessNames[i] = (char*) malloc(sizeof(char)*20);

      InterruptHandlerPC = R[REG_A0];
    }

    break;
  }

  // this syscall assigns the register value in R [27] to the PC. 
  // And the program continues from there.
  case SET_PC:
  {
    waitpid_signal = false;
    PC = R[27];

    break;
  }

/*  
  Preserves the label's address where the kernel should come out. 
  Thus, when all programs are finished, 
  the kernel goes to this label and the program ends.*/
  case SET_EXIT_PC:
  {
    if (EXIT_PC == 0)
    {
      EXIT_PC = R[REG_A0];
    }
    break;
  }

  case APPEND_PROCESS_NAME:
  {
    char* temp = ((char *)mem_reference(R[REG_A0]));
    strcpy(ProcessNames[numberOfProcessName], temp);
    numberOfProcessName++;
    break;
  }

  default:
    run_error("Unknown system call: %d\n", R[REG_V0]);
    break;
  }

#ifdef _WIN32
  windowsParameterHandlingControl(1);
#endif
  return (1);
}

void handle_exception()
{
  if (!quiet && CP0_ExCode != ExcCode_Int)
    error("Exception occurred at PC=0x%08x\n", CP0_EPC);

  exception_occurred = 0;
  PC = EXCEPTION_ADDR;

  switch (CP0_ExCode)
  {
  case ExcCode_Int:
    break;

  case ExcCode_AdEL:
    if (!quiet)
      error("  Unaligned address in inst/data fetch: 0x%08x\n", CP0_BadVAddr);
    break;

  case ExcCode_AdES:
    if (!quiet)
      error("  Unaligned address in store: 0x%08x\n", CP0_BadVAddr);
    break;

  case ExcCode_IBE:
    if (!quiet)
      error("  Bad address in text read: 0x%08x\n", CP0_BadVAddr);
    break;

  case ExcCode_DBE:
    if (!quiet)
      error("  Bad address in data/stack read: 0x%08x\n", CP0_BadVAddr);
    break;

  case ExcCode_Sys:
    if (!quiet)
      error("  Error in syscall\n");
    break;

  case ExcCode_Bp:
    exception_occurred = 0;
    return;

  case ExcCode_RI:
    if (!quiet)
      error("  Reserved instruction execution\n");
    break;

  case ExcCode_CpU:
    if (!quiet)
      error("  Coprocessor unuable\n");
    break;

  case ExcCode_Ov:
    if (!quiet)
      error("  Arithmetic overflow\n");
    break;

  case ExcCode_Tr:
    if (!quiet)
      error("  Trap\n");
    break;

  case ExcCode_FPE:
    if (!quiet)
      error("  Floating point\n");
    break;

  default:
    if (!quiet)
      error("Unknown exception: %d\n", CP0_ExCode);
    break;
  }
}