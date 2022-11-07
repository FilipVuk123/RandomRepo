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

    printf("Executing remote command...\n");
    rc = ssh_channel_request_exec(channel, "sh start_stream_script.sh -p 1234 -a 192.168.1.1");
    if (rc != SSH_OK)
    {
        free_channel(channel);
        error(session);
    }
    else
    {
        printf("Command executed!\n");
    }

    sleep(5);

    free_channel(channel);
    free_session(session);

    return 0;
}
