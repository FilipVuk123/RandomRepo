#include <libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void free_channel(ssh_channel channel)
{
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

void free_session(ssh_session session)
{
    ssh_disconnect(session);
    ssh_free(session);
}

void error(ssh_session session)
{
    fprintf(stderr, "Error: %s\n", ssh_get_error(session));
    free_session(session);
    exit(-1);
}

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("Usage: ./program <ip>");
        return 0;
    }

    ssh_session session;
    ssh_channel channel;
    int rc, port = 22;
    char buffer[1024];
    unsigned int nbytes;

    printf("Session...\n");
    session = ssh_new();
    if (session == NULL)
        exit(-1);

    ssh_options_set(session, SSH_OPTIONS_HOST, argv[1]);
    ssh_options_set(session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(session, SSH_OPTIONS_USER, "root");

    printf("Connecting...\n");
    rc = ssh_connect(session);
    if (rc != SSH_OK)
        error(session);

    printf("Password Autentication...\n");
    rc = ssh_userauth_password(session, NULL, "root");
    if (rc != SSH_AUTH_SUCCESS)
        error(session);

    printf("Channel...\n");
    channel = ssh_channel_new(session);
    if (channel == NULL)
        exit(-1);

    printf("Opening...\n");
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK)
        error(session);

    printf("Executing pidof command...\n");
    rc = ssh_channel_request_exec(channel, "pidof stream");
    if (rc != SSH_OK)
    {
        free_channel(channel);
        error(session);
    }

    printf("Received:\n");
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    if (nbytes > 0)
        printf("%s\n", buffer);


    free_channel(channel);
    free_session(session);



    /*********************************/

    ssh_session session1;
    ssh_channel channel1;

    printf("Session...\n");
    session1 = ssh_new();
    if (session1 == NULL)
        exit(-1);

    ssh_options_set(session1, SSH_OPTIONS_HOST, argv[1]);
    ssh_options_set(session1, SSH_OPTIONS_PORT, &port);
    ssh_options_set(session1, SSH_OPTIONS_USER, "root");

    printf("Connecting...\n");
    rc = ssh_connect(session1);
    if (rc != SSH_OK)
        error(session1);

    printf("Password Autentication...\n");
    rc = ssh_userauth_password(session1, NULL, "root");
    if (rc != SSH_AUTH_SUCCESS)
        error(session1);

    printf("Channel...\n");
    channel1 = ssh_channel_new(session1);
    if (channel == NULL)
        exit(-1);

    printf("Opening...\n");
    rc = ssh_channel_open_session(channel1);
    if (rc != SSH_OK)
        error(session1);


    printf("Executing kill command...\n");
    // sending SIGINT
    char kill_command[] = "kill -INT ";
    strcat(kill_command, buffer);
    printf("%s\n", kill_command);
    rc = ssh_channel_request_exec(channel1, kill_command);
    printf("Here!\n");
    if (rc != SSH_OK)
    {
        free_channel(channel1);
        error(session1);
    }
    else
    {
        printf("Program DONE!\n");
    }

    free_channel(channel1);
    free_session(session1);

    return 0;
}
