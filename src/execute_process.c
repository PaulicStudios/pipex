/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execute_process.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pgrossma <pgrossma@student.42heilbronn.de> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 20:56:31 by pgrossma          #+#    #+#             */
/*   Updated: 2024/02/21 17:49:08 by pgrossma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

bool	ft_execute_process(t_process *process, char **envp)
{
	process->pid = fork();
	if (process->pid == -1)
		return (false);
	if (process->pid == 0)
	{
		if (dup2(process->pipe_fd_in[PIPE_READ], STDIN_FILENO) == -1)
			return (false);
		close(process->pipe_fd_in[PIPE_READ]);
		close(process->pipe_fd_in[PIPE_WRITE]);
		if (dup2(process->pipe_fd_out[PIPE_WRITE], STDOUT_FILENO) == -1)
			return (false);
		close(process->pipe_fd_out[PIPE_READ]);
		close(process->pipe_fd_out[PIPE_WRITE]);
		execve(process->cmd, process->args, envp);
	}
	return (true);
}

void	ft_wait_for_processes(void)
{
	int	status;
	int	pid;

	pid = 1;
	while (pid > 0)
		pid = wait(&status);
}

void	ft_close_pipes(t_args *args)
{
	int	ind;

	if (!args->processes)
		return ;
	ind = 0;
	while (args->processes[ind])
	{
		close(args->processes[ind]->pipe_fd_in[0]);
		close(args->processes[ind]->pipe_fd_in[1]);
		close(args->processes[ind]->pipe_fd_out[0]);
		close(args->processes[ind]->pipe_fd_out[1]);
		ind++;
	}
}

void	ft_init_process(int ind, t_args *args, t_process *process, char **envp)
{
	if (pipe(args->processes[ind]->pipe_fd_out) != 0)
		ft_exit_error(args, "Could not create pipe");
	args->processes[ind]->pipe_fd_in[PIPE_READ]
		= args->processes[ind - 1]->pipe_fd_out[PIPE_READ];
	args->processes[ind]->pipe_fd_in[PIPE_WRITE]
		= args->processes[ind - 1]->pipe_fd_out[PIPE_WRITE];
	ft_execute_process(args->processes[ind], envp);
	close(args->processes[ind]->pipe_fd_in[PIPE_READ]);
	close(args->processes[ind]->pipe_fd_in[PIPE_WRITE]);
}

void	ft_execute_processes(t_args *args, char **envp)
{
	int	ind;

	ind = 0;
	while (args->processes[ind])
	{
		if (ind == 0)
		{
			if (pipe(args->processes[ind]->pipe_fd_out) != 0)
				ft_exit_error(args, "Could not create pipe");
			args->processes[ind]->pipe_fd_in[PIPE_READ] = args->fd_in;
			ft_execute_process(args->processes[ind], envp);
		}
		else if (ind == args->process_len - 1)
		{
			args->processes[ind]->pipe_fd_in[PIPE_READ]
				= args->processes[ind - 1]->pipe_fd_out[PIPE_READ];
			args->processes[ind]->pipe_fd_in[PIPE_WRITE]
				= args->processes[ind - 1]->pipe_fd_out[PIPE_WRITE];
			args->processes[ind]->pipe_fd_out[PIPE_WRITE] = args->fd_out;
			ft_execute_process(args->processes[ind], envp);
		}
		else
			ft_init_process(ind, args, args->processes[ind], envp);
		ind++;
	}
}
