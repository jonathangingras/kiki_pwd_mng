#include <stdio.h>
#include <blurgather/context.h>

#define ERROR_AND_RETURN(ret, msg, ...) fprintf(stderr, msg, ##__VA_ARGS__); return ret


int blur_create_context(bg_context **_ctx,
                        bg_persister_t *persister, bg_repository_t *repo, bg_cryptor_t *cryptor) {
  int err = 0;
  if((err = bgctx_init(_ctx))) {
    ERROR_AND_RETURN(-1, "could not instantiate blurgather context: %d\n", err);
  }

  bg_context *ctx = *_ctx;
  if((err = bgctx_register_persister(ctx, persister))) {
    ERROR_AND_RETURN(-2, "could not register persister: %d\n", err);
  }
  if((err = bgctx_register_repository(ctx, repo))) {
    ERROR_AND_RETURN(-3, "could not register repository: %d\n", err);
  }
  if((err = bgctx_register_cryptor(ctx, cryptor))) {
    ERROR_AND_RETURN(-4, "could not register cryptor: %d\n", err);
  }
  if((err = bgctx_config(ctx, BGCTX_ACQUIRE_PERSISTER | BGCTX_ACQUIRE_REPOSITORY))) {
    ERROR_AND_RETURN(-5, "could not configurate context: %d\n", err);
  }
  if((err = bgctx_seal(ctx))) {
    ERROR_AND_RETURN(-6, "could not seal context: %d\n", err);
  }

  if((err = bgctx_load(ctx))) {
    if(err == -4) {
      fprintf(stdout, "could not load repository (err %d), creating empty one.\n", err);
    } else {
      fprintf(stdout, "loading repository failed (err %d)!\n", err);
      return err;
    }

    if((err = bgctx_persist(ctx))) {
      fprintf(stderr, "could not persist repository!\n");
      return err;
    }
  }

  return err;
}
