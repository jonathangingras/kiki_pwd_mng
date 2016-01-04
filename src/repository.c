#include "password_repository.h"

void bg_pwd_repository_destroy(bg_password_repository* self) {
  self->vtable->destroy(self);
}

int bg_pwd_repository_add(bg_password_repository* self, bg_password* password) {
  return self->vtable->add(self, password);
}

int bg_pwd_repository_remove(bg_password_repository* self, const char* name) {
  return self->vtable->remove(self, name);
}

void bg_pwd_repository_sort(bg_password_repository* self) {
  self->vtable->sort(self);
}

bg_password_iterator bg_pwd_repository_begin(bg_password_repository* self) {
  return self->vtable->begin(self);
}

bg_password_iterator bg_pwd_repository_end(bg_password_repository* self) {
  return self->vtable->end(self);
}

int bg_pwd_repository_load(bg_password_repository* self) {
  return self->vtable->load(self);
}

int bg_pwd_repository_persist(bg_password_repository* self) {
  return self->vtable->persist(self);
}
