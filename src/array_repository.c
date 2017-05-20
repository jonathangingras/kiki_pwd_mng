#include <blurgather/array_repository.h>

static void bg_password_array_repository_destroy(bg_repository_t * _self);
static int bg_password_array_repository_add(bg_repository_t * self, bg_password* password);
static int bg_password_array_repository_get(bg_repository_t *self, const bg_string *name, bg_password **password);
static int bg_password_array_repository_remove(bg_repository_t * self, const bg_string *name);

static size_t bg_password_array_repository_count(bg_repository_t * _self);

static bg_password_iterator bg_password_array_repository_begin(bg_repository_t * _self);
static bg_password_iterator bg_password_array_repository_end(bg_repository_t * _self);

static bg_password** bg_password_array_repository_password_iterator_previous(bg_password_iterator* _self);
static bg_password** bg_password_array_repository_password_iterator_next(bg_password_iterator* _self);

static struct bg_repository_vtable bg_password_array_repository_vtable = {
  .destroy = &bg_password_array_repository_destroy,
  .add     = &bg_password_array_repository_add,
  .get     = &bg_password_array_repository_get,
  .remove  = &bg_password_array_repository_remove,
  .count   = &bg_password_array_repository_count,
  .begin   = &bg_password_array_repository_begin,
  .end     = &bg_password_array_repository_end,
};


bg_password_array_repository* bg_password_array_repository_init(bg_password_array_repository* _self, bg_context *ctx) {
	bg_password_array_repository* self = _self ? _self : (bg_password_array_repository*) bgctx_allocate(ctx, sizeof(bg_password_array_repository));

  self->ctx = ctx;
  self->repository.object = (void *) self;
	self->repository.vtable = &bg_password_array_repository_vtable;

	self->number_passwords = 0;
	self->password_array = (bg_password_array) calloc(25, sizeof(bg_password*));
	self->allocated_length = 25;

	return self;
}

static void bg_password_array_repositoryIterator_init(bg_password_array_repository* self, bg_password_iterator* iterator) {
	iterator->container = self;
	iterator->previous = &bg_password_array_repository_password_iterator_previous;
	iterator->next = &bg_password_array_repository_password_iterator_next;
}

bg_password_iterator bg_password_array_repository_begin(bg_repository_t * _self) {
	bg_password_array_repository* self = (bg_password_array_repository*) _self->object;

	bg_password_iterator iterator;
	bg_password_array_repositoryIterator_init(self, &iterator);
	iterator.value = &self->password_array[0];

	return iterator;
}

bg_password_iterator bg_password_array_repository_end(bg_repository_t * _self) {
	bg_password_array_repository* self = (bg_password_array_repository*) _self->object;

	bg_password_iterator iterator;
	bg_password_array_repositoryIterator_init(self, &iterator);
	iterator.value = &self->password_array[self->number_passwords];

	return iterator;
}

bg_password** bg_password_array_repository_password_iterator_previous(bg_password_iterator* self) {
	return --self->value;
}

bg_password** bg_password_array_repository_password_iterator_next(bg_password_iterator* self) {
	return ++self->value;
}

void bg_password_array_repository_free(bg_password_array_repository* self) {
	bg_repository_destroy(&self->repository);
	free(self);
}


/* methods */

void bg_password_array_repository_destroy(bg_repository_t *_self) {
  bg_password_array_repository* self = (bg_password_array_repository*) _self->object;

  size_t i;
  for(i = 0; i < self->number_passwords; ++i) {
    bg_password_free(self->password_array[i]);
  }
  free(self->password_array);
}

static bg_password* find_password_by_name(bg_password_array_repository* self, const bg_string *name, size_t *index_found) {
	bg_password* password = NULL;

	size_t i;
	for(i = 0; i < self->number_passwords; ++i) {
		if(bg_string_compare(bg_password_name(self->password_array[i]), name) == 0) {
			password = self->password_array[i];
			if(index_found) {
				*index_found = i;
			}
      break;
		}
	}

	return password;
}

static int add_new_password(bg_password_array_repository* self, bg_password* password) {
  if(bg_string_empty(bg_password_name(password))) {
    return -2;
  }

	if(self->allocated_length <= self->number_passwords + 1) {
		self->password_array = (bg_password_array) realloc(self->password_array, self->allocated_length + 25);
		self->allocated_length += 25;
	}

	self->password_array[self->number_passwords] = password;
	self->number_passwords++;
	return 0;
}

int bg_password_array_repository_add(bg_repository_t * _self, bg_password* _password) {
	bg_password_array_repository* self = (bg_password_array_repository*) _self->object;

	bg_password* password = find_password_by_name(self, bg_password_name(_password), NULL);

	if(!password) {
		return add_new_password(self, _password);
	}
	return -1;
}

int bg_password_array_repository_get(bg_repository_t *_self, const bg_string *name, bg_password **password) {
  bg_password_array_repository* self = (bg_password_array_repository*) _self->object;

  bg_password *found = find_password_by_name(self, name, NULL);
  *password = found;

  return found == NULL;
}

int bg_password_array_repository_remove(bg_repository_t * _self, const bg_string *name) {
	bg_password_array_repository* self = (bg_password_array_repository*) _self->object;

	bg_password* pwd = NULL;
  size_t password_index;
	if((pwd = find_password_by_name(self, name, &password_index)) != NULL) {

    self->password_array[password_index] = NULL;

		if(password_index != self->number_passwords - 1 && self->number_passwords > 1) {
			self->password_array[password_index] = self->password_array[self->number_passwords - 1];
			self->password_array[self->number_passwords - 1] = NULL;
		}

		self->number_passwords--;
    bg_password_free(pwd);

		return 0;
	}
	return -1;
}

static int compare_passwords(const void* _pass1, const void* _pass2) {
	bg_password* pass1 = *(bg_password**) _pass1, * pass2 = *(bg_password**) _pass2;
	return bg_string_compare(bg_password_name(pass1), bg_password_name(pass2));
}

void bg_password_array_repository_sort(bg_repository_t * _self) {
	bg_password_array_repository *self = (bg_password_array_repository*) _self->object;

	if(self->number_passwords < 2) return;

	qsort(self->password_array, self->number_passwords, sizeof(bg_password*), &compare_passwords);
}

size_t bg_password_array_repository_count(bg_repository_t * _self) {
  bg_password_array_repository *self = (bg_password_array_repository*) _self->object;

  return self->number_passwords;
}

bg_repository_t *bg_password_array_repository_repository(bg_password_array_repository *msgpack_persister) {
  return &msgpack_persister->repository;
}
