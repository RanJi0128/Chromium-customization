#define BOOL(x)                    COLUMN("numeric(1)", bool, x, 0, 0)
#define BOOL_ARRAY(x, items)       COLUMN_ARRAY("numeric(1)", bool, x, 0, 0, items)
#define BOOL_(x)                   COLUMN_("numeric(1)", bool, x, 0, 0)
#define BOOL_ARRAY_(x, items)      COLUMN_ARRAY_("numeric(1)", bool, x, 0, 0, items)

#define INT(x)                     COLUMN("integer", int, x, 0, 0)
#define INT_ARRAY(x, items)        COLUMN_ARRAY("integer", int, x, 0, 0, items)
#define INT_(x)                    COLUMN_("integer", int, x, 0, 0)
#define INT_ARRAY_(x, items)       COLUMN_ARRAY("integer", int, x, 0, 0, items)

#define INT64(x)                   COLUMN("bigint", int64, x, 0, 0)
#define INT64_ARRAY(x, items)      COLUMN_ARRAY("bigint", int64, x, 0, 0, items)
#define INT64_(x)                  COLUMN_("bigint", int64, x, 0, 0)
#define INT64_ARRAY_(x, items)     COLUMN_ARRAY("bigint", int64, x, 0, 0, items)

#define DOUBLE(x)                  COLUMN("double precision", double, x, 0, 0)
#define DOUBLE_ARRAY(x, items)     COLUMN_ARRAY("double precision", double, x, 0, 0, items)
#define DOUBLE_(x)                 COLUMN_("double precision", double, x, 0, 0)
#define DOUBLE_ARRAY_(x, items)    COLUMN_ARRAY_("double precision", double, x, 0, 0, items)

#define STRING(x, n)               COLUMN("varchar(" #n ")", String, x, n, 0)
#define STRING_ARRAY(x, n, items)  COLUMN_ARRAY("varchar(" #n ")", String, x, n, 0, items)
#define STRING_(x, n)              COLUMN_("varchar(" #n ")", String, x, n, 0)
#define STRING_ARRAY_(x, n, items) COLUMN_ARRAY_("varchar(" #n ")", String, x, n, 0, items)

#define DATE(x)                    COLUMN("date", Date, x, 0, 0)
#define DATE_ARRAY(x, items)       COLUMN_ARRAY("date", Date, x, 0, 0, items)
#define DATE_(x)                   COLUMN_("date", Date, x, 0, 0)
#define DATE_ARRAY_(x, items)      COLUMN_ARRAY_("date", Date, x, 0, 0, items)

#define TIME(x)                    COLUMN("timestamp", Time, x, 0, 0)
#define TIME_ARRAY(x, items)       COLUMN_ARRAY("timestamp", Time, x, 0, 0, items)
#define TIME_(x)                   COLUMN_("timestamp", Time, x, 0, 0)
#define TIME_ARRAY_(x, items)      COLUMN_ARRAY_("timestamp", Time, x, 0, 0, items)

#define BLOB(x)                    COLUMN("blob", String, x, 0, 0)
#define BLOB_(x)                   COLUMN_("blob", String, x, 0, 0)

#define SEQUENCE(x)                SCHEMA("create sequence " #x "; alter sequence " #x " restart with 1;",\
                                          "drop sequence " #x ";") \
                                   UPGRADE("create sequence " #x ";")
#define SEQUENCE_(x)               DOID(x) SEQUENCE(x)

#ifndef PRIMARY_KEY
#define PRIMARY_KEY                INLINE_ATTRIBUTE("primary key")
#endif

#define NOT_NULL                   INLINE_ATTRIBUTE("not null")
#define UNIQUE                     INLINE_ATTRIBUTE("unique")
#define SQLDEFAULT(v)              INLINE_ATTRIBUTE("default " v)

#define INDEX                      ATTRIBUTE("create index IDX_@x on @t(@c);", \
                                             "drop index IDX_@x;")
#ifndef REFERENCES
#define REFERENCES(x)              ATTRIBUTE("alter table @t add (constraint FK_@x foreign key "\
                                             "(@c) references " #x ");",\
                                             "alter table @t drop constraint FK_@x;")
#endif
#ifndef REFERENCES_CASCADE
#define REFERENCES_CASCADE(x)      ATTRIBUTE("alter table @t add (constraint FK_@x foreign key "\
                                             "(@c) references " #x " on delete cascade);",\
                                             "alter table @t drop constraint FK_@x;")
#endif
#ifndef REFERENCES_
#define REFERENCES_(n, x)          ATTRIBUTE("alter table @t add (constraint FK_@x$" #n " foreign key "\
                                             "(@c) references " #x ");",\
                                             "alter table @t drop constraint FK_@x$" #n ";")
#endif
#ifndef REFERENCES_CASCADE_
#define REFERENCES_CASCADE_(n, x)  ATTRIBUTE("alter table @t add (constraint FK_@x$" #n " foreign key "\
                                             "(@c) references " #x " on delete cascade);",\
                                             "alter table @t drop constraint FK_@x$" #n ";")
#endif

#define DUAL_PRIMARY_KEY(k1, k2)   INLINE_ATTRIBUTE(", primary key (" #k1 ", " #k2 ")")

#define DUAL_UNIQUE(k1, k2)        ATTRIBUTE("alter table @t add constraint DQ_@t unique "\
                                             "(" #k1 ", " #k2 ");",\
                                             "alter table @t drop constraint DQ_@t;")

#define UNIQUE_LIST(u, l)          ATTRIBUTE("alter table @t add constraint UQ_@t$" #u " unique "\
                                             "(" l ");",\
                                             "alter table @t drop constraint UQ_@t$" #u ";")

#define SQLCHECK(n, ct)            ATTRIBUTE("alter table @t add constraint CHK_@t$" #n " check "\
                                             "(" ct ");",\
                                             "alter table @t drop constraint CHK_@t$" #n ";")

#include <Sql/sch_model.h>


#undef BOOL
#undef BOOL_ARRAY
#undef BOOL_
#undef BOOL_ARRAY_

#undef INT
#undef INT_ARRAY
#undef INT_
#undef INT_ARRAY_

#undef INT64
#undef INT64_ARRAY
#undef INT64_
#undef INT64_ARRAY_

#undef DOUBLE
#undef DOUBLE_ARRAY
#undef DOUBLE_
#undef DOUBLE_ARRAY_

#undef STRING
#undef STRING_ARRAY
#undef STRING_
#undef STRING_ARRAY_

#undef DATE
#undef DATE_ARRAY
#undef DATE_
#undef DATE_ARRAY_

#undef TIME
#undef TIME_ARRAY
#undef TIME_
#undef TIME_ARRAY_

#undef BLOB
#undef BLOB_

#undef SEQUENCE

#undef PRIMARY_KEY
#undef NOT_NULL
#undef INDEX
#undef UNIQUE
#undef SQLDEFAULT
#undef REFERENCES
#undef REFERENCES_
#undef REFERENCES_CASCADE
#undef REFERENCES_CASCADE_
#undef DUAL_PRIMARY_KEY
#undef DUAL_UNIQUE
#undef UNIQUE_LIST
#undef SQLCHECK
