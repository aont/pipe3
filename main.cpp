#include <cstdio>
#include <csignal>
#include <sys/wait.h>

class pipe3
{

private:
  struct fd_pair
  {
    int read, write;
    int* get() { return (int*)this; }
    const int* get() const { return (const int*)this; }
  };

private:
  int fd_stdin;
  int fd_stdout;
  int fd_stderr;
  int pid;

public:
  int get_stdin() { return this->fd_stdin; }
  int get_stdout() { return this->fd_stdout; }
  int get_stderr() { return this->fd_stderr; }
  int get_pid() { return this->pid; }

public:
  int open(const char* const command)
  {

    fd_pair pipe_stdin;
    fd_pair pipe_stdout;
    fd_pair pipe_stderr;

    if(pipe(pipe_stdin.get())<0){
      perror("opening pipe_stdin failed.");
      throw;
    } else if(pipe(pipe_stdout.get())<0){
      perror("opening pipe_stdout failed.");
      ::close(pipe_stdin.read);
      ::close(pipe_stdin.write);
      throw;
    }
    else if(pipe(pipe_stderr.get())<0){
      perror("opening pipe_stderr failed.");
      ::close(pipe_stdin.read);
      ::close(pipe_stdin.write);
      ::close(pipe_stdout.read);
      ::close(pipe_stdout.write);
      throw;
    }



    /* Invoke processs */
    const int pid = fork();

    if(pid<0){ // fail

      perror("fork() failed.");

      ::close(pipe_stdin.read);
      ::close(pipe_stdin.write);

      ::close(pipe_stdout.read);
      ::close(pipe_stdout.write);

      ::close(pipe_stderr.read);
      ::close(pipe_stderr.write);

      throw;

    }else if(pid==0){   /* I'm child */

      ::close(pipe_stdin.write);
      ::close(pipe_stdout.read);
      ::close(pipe_stderr.read);
      
      ::dup2(pipe_stdin.read, 0);
      ::dup2(pipe_stdout.write, 1);
      ::dup2(pipe_stderr.write, 2);

      ::close(pipe_stdin.read);
      ::close(pipe_stdout.write);
      ::close(pipe_stderr.write);

      if(execlp(command, "", NULL)<0){
	perror("execlp failed.");

	::close(pipe_stdin.read);
	::close(pipe_stdout.write);
	::close(pipe_stderr.write);

	throw;
      }

    } else { // parent

      this->pid = pid;

      ::close(pipe_stdin.read);
      ::close(pipe_stdout.write);
      ::close(pipe_stderr.write);
      
      fd_stdin = pipe_stdin.write;
      fd_stdout = pipe_stdout.read;
      fd_stderr = pipe_stderr.read;
      
      return pid;
    }
    
  }

  int kill(int sig=SIGKILL)
  {
    ::kill(this->pid, sig);
  }

  int close()
  {
    ::close(this->fd_stdin);
    ::close(this->fd_stdout);
    ::close(this->fd_stderr);

    int status;
    ::waitpid(this->pid, &status, 0);
    return status;

  }
};


int main()
{
  pipe3 p;
  p.open("./test.sh");

  FILE* const fp_pipe = fdopen(p.get_stdout(),"r");

  char buf[1024];

  fgets(buf, sizeof(buf), fp_pipe);
  printf("%s", buf);

  fgets(buf, sizeof(buf), fp_pipe);
  printf("%s", buf);

  fgets(buf, sizeof(buf), fp_pipe);
  printf("%s", buf);

  p.close();


  //fputc('\n', stdout);
  
  return 0;
}
