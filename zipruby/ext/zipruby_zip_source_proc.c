#include <string.h>

#include "zip.h"
#include "zipint.h"
#include "zipruby_zip_source_proc.h"
#include "ruby.h"

static VALUE proc_call(VALUE proc) {
  return rb_funcall(proc, rb_intern("call"), 0);
}

static ssize_t read_proc(void *state, void *data, size_t len, enum zip_source_cmd cmd) {
  struct read_proc *z;
  VALUE src;
  char *buf;
  size_t n;

  z = (struct read_proc *) state;
  buf = (char *) data;

  switch (cmd) {
  case ZIP_SOURCE_OPEN:
    return 0;

  case ZIP_SOURCE_READ:
    src = rb_protect(proc_call, z->proc, NULL);

    if (TYPE(src) != T_STRING) {
      return 0;
    }

    n = RSTRING_LEN(src);

    if (n > 0) {
      n = (n > len) ? len : n;
      memcpy(buf, RSTRING_PTR(src), n);
    }

    return n;

  case ZIP_SOURCE_CLOSE:
    return 0;

  case ZIP_SOURCE_STAT:
    {
      struct zip_stat *st = (struct zip_stat *)data;
      zip_stat_init(st);
      st->mtime = NUM2LONG(rb_funcall(z->mtime, rb_intern("tv_sec"), 0));
      return sizeof(*st);
    }

  case ZIP_SOURCE_ERROR:
    return 0;

  case ZIP_SOURCE_FREE:
    free(z);
    return 0;
  }

  return -1;
}

struct zip_source *zip_source_proc(struct zip *za, struct read_proc *z) {
  struct zip_source *zs;
  zs = zip_source_function(za, read_proc, z);
  return zs;
}
