#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <ocr.h>

#include "command.h"

static void add_arg(const char* long_option, const char short_option,
                    u8 has_arg, const char type, void* data, int size,
                    const char* help, command* cmd)
{
   option* o = cmd->options+cmd->options_num;
   strcpy(o->help, help);
   strcpy(o->long_arg, long_option);
   o->short_arg = short_option;
   cmd->args[cmd->args_num] = short_option;
   ++cmd->args_num;
   cmd->long_opt[cmd->options_num].name = o->long_arg;
   cmd->long_opt[cmd->options_num].has_arg = has_arg;
   cmd->long_opt[cmd->options_num].flag = NULL;
   if(has_arg) {
     cmd->args[cmd->args_num] = ':';
     ++cmd->args_num;
   }
   cmd->args[cmd->args_num] = '\0';
   o->type = type;
   o->ptr = data;
   o->sz = size;
   cmd->longest = cmd->longest > strlen(long_option) ? cmd->longest : strlen(long_option);
   ++cmd->options_num;
}

static void print_args(command* cmd)
{
  char s[16];
  PRINTF("\n  Arguments are: \n");
#ifdef TG_ARCH
  strcpy(s,"   --%%-9s");
#else
  sprintf(s,"   --%%-%us", cmd->longest);
#endif
  int o;
  for(o = 0; o < cmd->options_num; ++o) {
    PRINTF(s,cmd->options[o].long_arg);
    PRINTF(" -%c  arg=%1d type=%c  %s\n", cmd->options[o].short_arg, cmd->long_opt[o].has_arg,
                                          cmd->options[o].type, cmd->options[o].help);
  }
  PRINTF("\n\n");
}

static int find_option(option* o, u32 o_num, char c)
{
  u32 i;
  for(i = 0; i < o_num && o[i].short_arg != c; ++i);
  return i;
}

static void process_args(int argc, char** argv, command* cmd)
{
  while(1) {
    int option_index = 0;
    int c = getopt_long(argc, argv, cmd->args, cmd->long_opt, &option_index);
    if(c == -1) break;
    option_index = find_option(cmd->options, cmd->options_num, c);
    if(option_index == cmd->options_num) {
      PRINTF("\n\ninvalid switch : -%c in getopt()\n\n", c);
      continue;
    }

    if(!cmd->long_opt[option_index].has_arg)
      *(int*)cmd->options[option_index].ptr = 1;
    else {
      switch(cmd->options[option_index].type) {
      case 'i':
#ifdef TG_ARCH
        {
        int i = 0; int j = 0;
        while(optarg[i]>='0'&&optarg[i]<='9') {
          j*=10; j+= optarg[i]-'0'; ++i;
        }
        *(int*)cmd->options[option_index].ptr = j;
        }
#else
        sscanf(optarg,"%d",(int*)cmd->options[option_index].ptr);
#endif
        break;
      case 'f':
#ifdef TG_ARCH
        {
        int i = 0; int j = 0;
        while(optarg[i]>='0'&&optarg[i]<='9') {
          j*=10; j+= optarg[i]-'0'; ++i;
        }
        if(optarg[i]=='.') ++i;
        float k = 0; float l = 0.1;
        while(optarg[i]>='0'&&optarg[i]<='9') {
          k+= (optarg[i]-'0')*l; l*=0.1; ++i;
        }
        *(float*)cmd->options[option_index].ptr = j+k;
        }
#else
        sscanf(optarg,"%f",(float*)cmd->options[option_index].ptr);
#endif
        break;
      case 'd':
#ifdef TG_ARCH
        {
        int i = 0; int j = 0;
        while(optarg[i]>='0'&&optarg[i]<='9') {
          j*=10; j+= optarg[i]-'0'; ++i;
        }
        if(optarg[i]=='.') ++i;
        double k = 0; double l = 0.1;
        while(optarg[i]>='0'&&optarg[i]<='9') {
          k+= (optarg[i]-'0')*l; l*=0.1; ++i;
        }
        *(double*)cmd->options[option_index].ptr = j+k;
        }
#else
        sscanf(optarg,"%lf",(double*)cmd->options[option_index].ptr);
#endif
        break;
      case 's':
        strncpy((char*)cmd->options[option_index].ptr,(char*)optarg,cmd->options[option_index].sz);
        break;
      case 'c':
        *(char*)cmd->options[option_index].ptr = *optarg;
        break;
      default:
        PRINTF("\n\n    invalid type : %c in getopt()\n    valid values are"
               " 'e', 'z'. 'i','d','f','s', and 'c'\n\n\n", c);
      }
    }
  }
}

void parse_command(int argc, char** argv, void* c)
{
  command* cmd = (command*) c;
  strcpy(cmd->pot_dir,  "pots");
  strcpy(cmd->pot_name, "\0"); // default depends on pot_type
  strcpy(cmd->pot_type, "funcfl");
  cmd->doeam = 0;
  cmd->nx = 20;
  cmd->ny = 20;
  cmd->nz = 20;
  cmd->steps = 100;
  cmd->period = 10;
  cmd->dt = 1.0;
  cmd->lat = -1.0;
  cmd->temperature = 600.0;
  cmd->initial_delta = 0.0;

  cmd->options_num = 0;
  cmd->args_num = 0;
  cmd->longest = 1;

  u8 help=0;
  // add arguments for processing.  Please update the html documentation too!
  add_arg("help",     'h', 0, 'i', &help,                0,              "print this message", cmd);
  add_arg("pot_dir",  'd', 1, 's', cmd->pot_dir,  sizeof(cmd->pot_dir),  "potential directory", cmd);
  add_arg("pot_name", 'p', 1, 's', cmd->pot_name, sizeof(cmd->pot_name), "potential name", cmd);
  add_arg("pot_type", 't', 1, 's', cmd->pot_type, sizeof(cmd->pot_type), "potential type (funcfl or setfl)", cmd);
  add_arg("doeam",    'e', 0, 'i', &(cmd->doeam),         0,             "compute eam potentials", cmd);
  add_arg("nx",       'x', 1, 'i', &(cmd->nx),            0,             "number of unit cells in x", cmd);
  add_arg("ny",       'y', 1, 'i', &(cmd->ny),            0,             "number of unit cells in y", cmd);
  add_arg("nz",       'z', 1, 'i', &(cmd->nz),            0,             "number of unit cells in z", cmd);
  add_arg("steps",    'N', 1, 'i', &(cmd->steps),         0,             "number of time steps", cmd);
  add_arg("period",   'n', 1, 'i', &(cmd->period),        0,             "number of steps between output", cmd);
  add_arg("dt",       'D', 1, 'd', &(cmd->dt),            0,             "time step (in fs)", cmd);
  add_arg("lat",      'l', 1, 'd', &(cmd->lat),           0,             "lattice parameter (Angstroms)", cmd);
  add_arg("temp",     'T', 1, 'd', &(cmd->temperature),   0,             "initial temperature (K)", cmd);
  add_arg("delta",    'r', 1, 'd', &(cmd->initial_delta), 0,             "initial delta (Angstroms)", cmd);

  process_args(argc,argv,cmd);

  if(strlen(cmd->pot_name) == 0) {
    if(strcmp(cmd->pot_type, "setfl") == 0)
      strcpy(cmd->pot_name, "Cu01.eam.alloy");
    if(strcmp(cmd->pot_type, "funcfl") == 0)
      strcpy(cmd->pot_name, "Cu_u6.eam");
  }

  if(help)
    print_args(cmd);
  else
    PRINTF("Command Line Parameters:\n");
    PRINTF("  doeam: %u\n", cmd->doeam);
    PRINTF("  pot_dir: %s\n", cmd->pot_dir);
    PRINTF("  pot_name: %s\n", cmd->pot_name);
    PRINTF("  pot_type: %s\n", cmd->pot_type);
    PRINTF("  nx: %u\n", cmd->nx);
    PRINTF("  ny: %u\n", cmd->ny);
    PRINTF("  nz: %u\n", cmd->nz);
    PRINTF("  Lattice constant: %f Angstroms\n", cmd->lat);
    PRINTF("  Steps: %u\n",                      cmd->steps);
    PRINTF("  Period: %u\n",                     cmd->period);
    PRINTF("  Time step: %f fs\n",               cmd->dt);
    PRINTF("  Initial Temperature: %f K\n",      cmd->temperature);
    PRINTF("  Initial Delta: %f Angstroms\n",    cmd->initial_delta);
    PRINTF("\n");
}
