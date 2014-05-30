#include "pil_enter.h"

void pil_enter(int func, int num_args, ...)
{
	va_list argptr;
	va_start(argptr, num_args);

	int i;
	for (i = 0; i < num_args; i++)
	{
		union arg_t
		{
			int i;
			long long ll;
			float f;
			double d;
			char c;
			char *s;
		} Arg;

		// process the arguments for each library function
		switch (func)
		{
			case 0:
				Arg.i = va_arg(argptr, int);
				printf("%d\n", Arg.i);
				break;
			case 1:
				Arg.i = va_arg(argptr, int);
				printf("%d\n", Arg.i);
				break;
			default:
				printf("ERROR: invalid type\n");
				break;
		}
	}

	va_end(argptr);

	// call the appropriate library function
	switch (func)
	{
		case 2:
#ifdef PIL2C
			node_2();
			break;
#endif
#ifdef PIL2SWARM
/*
			node_2_Param input;
			node_2_Param_init(&input, &global, 1, 1, 1);
			node_2.swarm_enter(&input, NULL);
			break;
*/
#endif
		default:
			printf("ERROR: unknown function id (%d) in pil_enter() or unemplemented for specified backend\n", func);
			break;
	}
}
