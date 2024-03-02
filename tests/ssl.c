#include <openssl/ssl.h>
#include <openssl/err.h>

int main(int argc, char *argv[])
{
    SSL_CTX *ctx;
    SSL *ssl;

    SSL_load_error_strings();
    SSL_library_init();

    return 0;
}
