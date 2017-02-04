/**
 * Copyright 2009-2010 Bart Trojanowski <bart@jukie.net>
 * Licensed under GPLv2, or later, at your choosing.
 *
 * bidirectional popen() call
 *
 * @param rwepipe - int array of size three
 * @param exe - program to run
 * @param argv - argument list
 * @return pid or -1 on error
 *
 * The caller passes in an array of three integers (rwepipe), on successful
 * execution it can then write to element 0 (stdin of exe), and read from
 * element 1 (stdout) and 2 (stderr).
 */

#include "popenRWE.h"

int popenRWE(int* rwepipe, char const* command)
{
   int in[2];
   int out[2];
   int pid;
   int rc;

   rc = pipe(in);
   if (rc < 0)
      goto error_in;

   rc = pipe(out);
   if (rc < 0)
      goto error_out;

   pid = fork();
   if (pid > 0) { /* parent */
      close(in[0]);
      close(out[1]);
      rwepipe[0] = in[1];
      rwepipe[1] = out[0];
      return pid;
   }
   else if (pid == 0) { /* child */
      close(in[1]);
      close(out[0]);
      close(0);
      if (!dup(in[0])) {
         ;
      }
      close(1);
      if (!dup(out[1])) {
         ;
      }

      execl("/bin/sh", "sh", "-c", command, NULL);
      _exit(1);
   }
   else
      goto error_fork;

   return pid;

error_fork:
   close(out[0]);
   close(out[1]);
error_out:
   close(in[0]);
   close(in[1]);
error_in:
   return -1;
}

int pcloseRWE(int pid, int* rwepipe)
{
   int status;
   close(rwepipe[0]);
   close(rwepipe[1]);
   waitpid(pid, &status, 0);
   return status;
}