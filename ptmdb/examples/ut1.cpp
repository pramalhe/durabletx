#include <cassert>

#include "db.h"
#include "options.h"

int main(void) {
    ptmdb::DB* db;
    ptmdb::Options options{};
    options.create_if_missing = true;
    ptmdb::Status status = ptmdb::DB::Open(options, "/tmp/testdb", &db);
    assert(status.ok());
    delete db;
}
