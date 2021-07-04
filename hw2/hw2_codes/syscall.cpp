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
#include <stdlib.h>

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
#include <time.h>

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

// Our Process Table...
typedef struct _process_table
{
  pid_t ProcessID;
  char *ProcessName;
  char *Process_State;
  int CurrentProgramCounter;
  int Start_PC;
  int End_PC;
  int Data_PC;
  int parent_process;

  reg_word CPR_Copy[4][32], CCR_Copy[4][32];
  reg_word _R_Copy[R_LENGTH];
  reg_word HI_Copy, L0_Copy;

} process_table;

process_table main_process_table[50];

int numberOfProcesses = 0;
int numberOfProcesses_const = 0;
int current_process = 0;
int numberOfTerminatedProcesses = 0;

char namesOfFiles[3][50] = {"BinarySearch.asm", "LinearSearch.asm", "Collatz.asm"};
int numberOfFiles = 3;
int isDelete = -1;

bool waitpid_signal = false;
bool isTerminated = false;
int count = 0;

void printProcessTable()
{

  for (int i = 0; i < numberOfProcesses; i++)
  {
    printf("\n\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n\n");

    printf("Index : %d\n\n", i);
    printf("ProcessID : %d\n", main_process_table[i].ProcessID);
    printf("ProcessName : %s\n", main_process_table[i].ProcessName);
    printf("Process_State : %s\n", main_process_table[i].Process_State);
    printf("CurrentProgramCounter : %d\n", main_process_table[i].CurrentProgramCounter);
    printf("Start_PC : %d\n", main_process_table[i].Start_PC);
    printf("End_PC : %d\n", main_process_table[i].End_PC);
    printf("Stack Pointer Address : %d\n", main_process_table[i]._R_Copy[REG_SP]);
    printf("parent_process : %d\n", main_process_table[i].parent_process);

    printf("\n\n-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-o-\n\n");
  }
}

/*You implement your handler here*/
void SPIM_timerHandler()
{

  if (waitpid_signal == false)
  {
    // Implement your handler..
    try
    {
      if (isTerminated == true)
      {
        //sometimes isTerminated has a wrong value
        //thus we should control the $v0 parameter to look for asm file is finished...
        if(R[REG_V0] != 30){
          //if it is not finished dont change anything. Keep going with same process
          return;
        }
        for (int i = current_process; i < numberOfProcesses; i++)
        {
          main_process_table[i] = main_process_table[i + 1];
        }

        numberOfProcesses--;
        isTerminated = false;

        if (numberOfProcesses == 1)
        {
          PC = main_process_table[0].CurrentProgramCounter;

          for (int i = 0; i < 4; i++)
          {
            for (int j = 0; j < 32; j++)
            {
              CPR[i][j] = main_process_table[0].CPR_Copy[i][j];
              CCR[i][j] = main_process_table[0].CCR_Copy[i][j];
            }
          }

          for (int i = 0; i < R_LENGTH; i++)
            R[i] = main_process_table[0]._R_Copy[i];

          HI = main_process_table[0].HI_Copy;
          LO = main_process_table[0].L0_Copy;

          current_process = 0;
          R[REG_A1] = 99;
          printProcessTable();
          return;
        }
        main_process_table[current_process].Process_State = "Running";
      }
      else
      {
        if (numberOfProcesses == 1)
        {
          PC = main_process_table[0].CurrentProgramCounter;

          for (int i = 0; i < 4; i++)
          {
            for (int j = 0; j < 32; j++)
            {
              CPR[i][j] = main_process_table[0].CPR_Copy[i][j];
              CCR[i][j] = main_process_table[0].CCR_Copy[i][j];
            }
          }

          for (int i = 0; i < R_LENGTH; i++)
            R[i] = main_process_table[0]._R_Copy[i];

          HI = main_process_table[0].HI_Copy;
          LO = main_process_table[0].L0_Copy;

          current_process = 0;
          R[REG_A1] = 99;
          printProcessTable();
          return;
        }

        if (current_process > numberOfProcesses - 1)
          current_process = 1;

        main_process_table[current_process].CurrentProgramCounter = PC;

        for (int i = 0; i < 4; i++)
        {
          for (int j = 0; j < 32; j++)
          {
            main_process_table[current_process].CPR_Copy[i][j] = CPR[i][j];
            main_process_table[current_process].CCR_Copy[i][j] = CCR[i][j];
          }
        }

        for (int i = 0; i < R_LENGTH; i++)
          main_process_table[current_process]._R_Copy[i] = R[i];

        main_process_table[current_process].HI_Copy = HI;
        main_process_table[current_process].L0_Copy = LO;

        main_process_table[current_process].Process_State = "Ready";

        current_process++;
        
        if (current_process > numberOfProcesses - 1)
          current_process = 1;

        PC = main_process_table[current_process].CurrentProgramCounter;

        for (int i = 0; i < 4; i++)
        {
          for (int j = 0; j < 32; j++)
          {
            CPR[i][j] = main_process_table[current_process].CPR_Copy[i][j];
            CCR[i][j] = main_process_table[current_process].CCR_Copy[i][j];
          }
        }

        for (int i = 0; i < R_LENGTH; i++)
          R[i] = main_process_table[current_process]._R_Copy[i];

        HI = main_process_table[current_process].HI_Copy;
        LO = main_process_table[current_process].L0_Copy;

        main_process_table[current_process].Process_State = "Running";
      }

      if (main_process_table[current_process].CurrentProgramCounter == 0)
      {

        while (current_process > 0 && main_process_table[current_process].CurrentProgramCounter == 0)
        {
          current_process--;
        }

        PC = main_process_table[current_process].CurrentProgramCounter;

        for (int i = 0; i < 4; i++)
        {
          for (int j = 0; j < 32; j++)
          {
            CPR[i][j] = main_process_table[current_process].CPR_Copy[i][j];
            CCR[i][j] = main_process_table[current_process].CCR_Copy[i][j];
          }
        }

        for (int i = 0; i < R_LENGTH; i++)
          R[i] = main_process_table[current_process]._R_Copy[i];

        HI = main_process_table[current_process].HI_Copy;
        LO = main_process_table[current_process].L0_Copy;
        main_process_table[current_process].Process_State = "Running";
        
      }
      printProcessTable();
      //throw logic_error("NotImplementedException\n");
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

  /* Syscalls for the source-language version of SPIM.  These are easier to
     use than the real syscall and are portable to non-MIPS operating
     systems. */

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

  case INIT_STATE:
  {

    srand(time(NULL));
    process_table init_process;
    init_process.ProcessID = 0;
    init_process.ProcessName = "init";
    init_process.Process_State = "Running";
    init_process.CurrentProgramCounter = PC;
    init_process.Start_PC = starting_address();
    init_process.End_PC = current_text_pc();
    //init_process.Data_PC = PC;
    init_process.parent_process = -1;

    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 32; j++)
      {
        init_process.CPR_Copy[i][j] = CPR[i][j];
        init_process.CCR_Copy[i][j] = CCR[i][j];
      }
    }

    for (int i = 0; i < R_LENGTH; i++)
      init_process._R_Copy[i] = R[i];

    init_process.HI_Copy = HI;
    init_process.L0_Copy = LO;

    main_process_table[numberOfProcesses] = init_process;

    numberOfProcesses++;
    numberOfProcesses_const++;
    break;
  }
  case _FORK_:
  {
    process_table temp_process;
    temp_process.ProcessID = numberOfProcesses_const;
    temp_process.ProcessName = main_process_table[numberOfProcesses - 1].ProcessName;
    temp_process.Process_State = main_process_table[numberOfProcesses - 1].Process_State;
    temp_process.CurrentProgramCounter = main_process_table[numberOfProcesses - 1].CurrentProgramCounter;
    temp_process.Start_PC = main_process_table[numberOfProcesses - 1].Start_PC;
    temp_process.End_PC = main_process_table[numberOfProcesses - 1].End_PC;
    temp_process.Data_PC = main_process_table[numberOfProcesses - 1].Data_PC;
    temp_process.parent_process = main_process_table[numberOfProcesses - 1].ProcessID;

    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 32; j++)
      {
        temp_process.CPR_Copy[i][j] = main_process_table[numberOfProcesses - 1].CPR_Copy[i][j];
        temp_process.CCR_Copy[i][j] = main_process_table[numberOfProcesses - 1].CCR_Copy[i][j];
      }
    }

    for (int i = 0; i < R_LENGTH; i++)
      temp_process._R_Copy[i] = main_process_table[numberOfProcesses - 1]._R_Copy[i];

    temp_process.HI_Copy = main_process_table[numberOfProcesses - 1].HI_Copy;
    temp_process.L0_Copy = main_process_table[numberOfProcesses - 1].L0_Copy;

    main_process_table[numberOfProcesses] = temp_process;

    numberOfProcesses++;
    numberOfProcesses_const++;
    break;
  }

  case _EXECVE_:
  {
    int index = R[REG_A0];
    if (R[REG_A1] == 99)
      R[REG_A1] == 99;
    
    if(index >= numberOfFiles){
      printf("Something went wrong!! Please try again...\n");
      printf("The input of selected file index is out of bound!!\n");
      printf("Selected index : %d, Number of files : %d\n", index, numberOfFiles);
      printf("PC: %d\n",PC);
      spim_return_value=0;
      return(0);
    }
    
    printf("\nindex of filename : %d\n", index);
    printf("\nname of file : %s\n", namesOfFiles[index]);

    main_process_table[numberOfProcesses - 1].ProcessName = namesOfFiles[index];
    main_process_table[numberOfProcesses - 1].Process_State = "Ready";

    main_process_table[numberOfProcesses - 1].CurrentProgramCounter = current_text_pc();
    main_process_table[numberOfProcesses - 1].Start_PC = current_text_pc();
    read_assembly_file(namesOfFiles[index]);
    main_process_table[numberOfProcesses - 1].End_PC = current_text_pc();

    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 32; j++)
      {
        main_process_table[numberOfProcesses - 1].CPR_Copy[i][j] = CPR[i][j];
        main_process_table[numberOfProcesses - 1].CCR_Copy[i][j] = CCR[i][j];
      }
    }

    for (int i = 0; i < R_LENGTH; i++)
      main_process_table[numberOfProcesses - 1]._R_Copy[i] = R[i];

    main_process_table[numberOfProcesses - 1].HI_Copy = HI;
    main_process_table[numberOfProcesses - 1].L0_Copy = LO;

    break;
  }

  case _WAITPID_:
  {
    if(waitpid_signal == false)
      waitpid_signal = true;
    else
      waitpid_signal = false;
    break;
  }

  case _TERMINATE_PROCESS:
  {

    printf("\n\nTerminated -> ProcessID : %d\n\n", main_process_table[current_process].ProcessID);

    isTerminated = true;
    numberOfTerminatedProcesses++;

    SPIM_timerHandler();

    if (numberOfTerminatedProcesses > numberOfProcesses_const - 1)
    {
      PC = main_process_table[0].CurrentProgramCounter;

      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 32; j++)
        {
          CPR[i][j] = main_process_table[0].CPR_Copy[i][j];
          CCR[i][j] = main_process_table[0].CCR_Copy[i][j];
        }
      }

      for (int i = 0; i < R_LENGTH; i++)
        R[i] = main_process_table[0]._R_Copy[i];

      HI = main_process_table[0].HI_Copy;
      LO = main_process_table[0].L0_Copy;

      current_process = 0;
      R[REG_A1] = 99;
    }

    break;
  }

  case _RANDOM_NUMBER_GENERATOR:
  {
    // this syscall will create random number and it will assign it to a0 register
    // NOTE : THIS SYSCALL WILL NOT GENERATE NUMBER THAT IS BIGGER THAN R[REG_A0]'S VALUE...

    int maxBound = R[REG_A0];
    int randomNumber = rand() % maxBound;

    R[REG_A0] = randomNumber;

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
