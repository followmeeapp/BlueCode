//
//  main.cpp
//  Follow
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include <iostream>
#include <string>
#include <thread>

using namespace std;

#include "uWebSockets/uWS.h"
#include "lmdb/lmdb/lmdb.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "addressbook.capnp.h"

#define E(expr) CHECK((rc = (expr)) == MDB_SUCCESS, #expr)
#define RES(err, expr) ((rc = expr) == (err) || (CHECK(!rc, #expr), 0))
#define CHECK(test, msg) ((test) ? (void)0 : ((void)fprintf(stderr, \
	"%s:%d: %s: %s\n", __FILE__, __LINE__, msg, mdb_strerror(rc)), abort()))

int connections = 0;

int main()
{
    ::capnp::MallocMessageBuilder message;

    AddressBook::Builder addressBook = message.initRoot<AddressBook>();
    ::capnp::List<Person>::Builder people = addressBook.initPeople(2);

    Person::Builder alice = people[0];
    alice.setId(123);
    alice.setName("Alice");
    alice.setEmail("alice@example.com");
    // Type shown for explanation purposes; normally you'd use auto.
    ::capnp::List<Person::PhoneNumber>::Builder alicePhones = alice.initPhones(1);
    alicePhones[0].setNumber("555-1212");
    alicePhones[0].setType(Person::PhoneNumber::Type::MOBILE);
    alice.getEmployment().setSchool("MIT");

    Person::Builder bob = people[1];
    bob.setId(456);
    bob.setName("Bob");
    bob.setEmail("bob@example.com");

    auto bobPhones = bob.initPhones(2);
    bobPhones[0].setNumber("555-4567");
    bobPhones[0].setType(Person::PhoneNumber::Type::HOME);
    bobPhones[1].setNumber("555-7654");
    bobPhones[1].setType(Person::PhoneNumber::Type::WORK);
    bob.getEmployment().setUnemployed();

    for (Person::Reader person : addressBook.getPeople()) {
      std::cout << person.getName().cStr() << ": " << person.getEmail().cStr() << std::endl;

      for (Person::PhoneNumber::Reader phone: person.getPhones()) {
        const char* typeName = "UNKNOWN";
        switch (phone.getType()) {
          case Person::PhoneNumber::Type::MOBILE: typeName = "mobile"; break;
          case Person::PhoneNumber::Type::HOME: typeName = "home"; break;
          case Person::PhoneNumber::Type::WORK: typeName = "work"; break;
        }
        std::cout << "  " << typeName << " phone: " << phone.getNumber().cStr() << std::endl;
      }

      Person::Employment::Reader employment = person.getEmployment();
      switch (employment.which()) {
        case Person::Employment::UNEMPLOYED:
          std::cout << "  unemployed" << std::endl;
          break;
        case Person::Employment::EMPLOYER:
          std::cout << "  employer: "
                    << employment.getEmployer().cStr() << std::endl;
          break;
        case Person::Employment::SCHOOL:
          std::cout << "  student at: "
                    << employment.getSchool().cStr() << std::endl;
          break;
        case Person::Employment::SELF_EMPLOYED:
          std::cout << "  self-employed" << std::endl;
          break;
      }
    }

    kj::Array<capnp::word> words = capnp::messageToFlatArray(message);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    int rc;
	MDB_env *env;
	MDB_txn *txn;
	MDB_dbi user_dbi;
    // int i = 0, j = 0;
    // MDB_val key, data;
    // MDB_stat mst;
    // MDB_cursor *cursor, *cur2;
    // MDB_cursor_op op;
    // int count;
    // int *values;
    // char sval[32] = "";
    //
    // srand(time(NULL));
    //
    //     count = (rand()%384) + 64;
    //     values = (int *)malloc(count*sizeof(int));
    //
    //     for(i = 0;i<count;i++) {
    //     values[i] = rand()%1024;
    //     }

	E(mdb_env_create(&env));
	E(mdb_env_set_maxreaders(env, 126)); // same as default
	E(mdb_env_set_maxdbs(env, 16));      // support 16 named databases
	E(mdb_env_set_mapsize(env, 4096 * 256 * 1024 * 1)); // 1GB memory-map
	E(mdb_env_open(env, "/Users/erich/Desktop/testdb", MDB_NOTLS, 0664));

	E(mdb_txn_begin(env, NULL, 0, &txn));
	E(mdb_dbi_open(txn, "user", MDB_CREATE | MDB_INTEGERKEY, &user_dbi));
    E(mdb_txn_commit(txn));

    mdb_dbi_close(env, user_dbi);
    mdb_env_close(env);

    // key.mv_size = sizeof(int);
    // key.mv_data = sval;
    //
    // printf("Adding %d values\n", count);
    //     for (i=0;i<count;i++) {
    //     sprintf(sval, "%03x %d foo bar", values[i], values[i]);
    //     /* Set <data> in each iteration, since MDB_NOOVERWRITE may modify it */
    //     data.mv_size = sizeof(sval);
    //     data.mv_data = sval;
    //     if (RES(MDB_KEYEXIST, mdb_put(txn, dbi, &key, &data, MDB_NOOVERWRITE))) {
    //         j++;
    //         data.mv_size = sizeof(sval);
    //         data.mv_data = sval;
    //     }
    //     }
    // if (j) printf("%d duplicates skipped\n", j);
    // E(mdb_txn_commit(txn));
    // E(mdb_env_stat(env, &mst));
    //
    // E(mdb_txn_begin(env, NULL, MDB_RDONLY, &txn));
    // E(mdb_cursor_open(txn, dbi, &cursor));
    // while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0) {
    //     printf("key: %p %.*s, data: %p %.*s\n",
    //         key.mv_data,  (int) key.mv_size,  (char *) key.mv_data,
    //         data.mv_data, (int) data.mv_size, (char *) data.mv_data);
    // }
    // CHECK(rc == MDB_NOTFOUND, "mdb_cursor_get");
    // mdb_cursor_close(cursor);
    // mdb_txn_abort(txn);
    //
    // j=0;
    // key.mv_data = sval;
    //     for (i= count - 1; i > -1; i-= (rand()%5)) {
    //     j++;
    //     txn=NULL;
    //     E(mdb_txn_begin(env, NULL, 0, &txn));
    //     sprintf(sval, "%03x ", values[i]);
    //     if (RES(MDB_NOTFOUND, mdb_del(txn, dbi, &key, NULL))) {
    //         j--;
    //         mdb_txn_abort(txn);
    //     } else {
    //         E(mdb_txn_commit(txn));
    //     }
    //     }
    //     free(values);
    // printf("Deleted %d values\n", j);
    //
    // E(mdb_env_stat(env, &mst));
    // E(mdb_txn_begin(env, NULL, MDB_RDONLY, &txn));
    // E(mdb_cursor_open(txn, dbi, &cursor));
    // printf("Cursor next\n");
    // while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0) {
    //     printf("key: %.*s, data: %.*s\n",
    //         (int) key.mv_size,  (char *) key.mv_data,
    //         (int) data.mv_size, (char *) data.mv_data);
    // }
    // CHECK(rc == MDB_NOTFOUND, "mdb_cursor_get");
    // printf("Cursor last\n");
    // E(mdb_cursor_get(cursor, &key, &data, MDB_LAST));
    // printf("key: %.*s, data: %.*s\n",
    //     (int) key.mv_size,  (char *) key.mv_data,
    //     (int) data.mv_size, (char *) data.mv_data);
    // printf("Cursor prev\n");
    // while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_PREV)) == 0) {
    //     printf("key: %.*s, data: %.*s\n",
    //         (int) key.mv_size,  (char *) key.mv_data,
    //         (int) data.mv_size, (char *) data.mv_data);
    // }
    // CHECK(rc == MDB_NOTFOUND, "mdb_cursor_get");
    // printf("Cursor last/prev\n");
    // E(mdb_cursor_get(cursor, &key, &data, MDB_LAST));
    //     printf("key: %.*s, data: %.*s\n",
    //         (int) key.mv_size,  (char *) key.mv_data,
    //         (int) data.mv_size, (char *) data.mv_data);
    // E(mdb_cursor_get(cursor, &key, &data, MDB_PREV));
    //     printf("key: %.*s, data: %.*s\n",
    //         (int) key.mv_size,  (char *) key.mv_data,
    //         (int) data.mv_size, (char *) data.mv_data);
    //
    // mdb_cursor_close(cursor);
    // mdb_txn_abort(txn);
    //
    // printf("Deleting with cursor\n");
    // E(mdb_txn_begin(env, NULL, 0, &txn));
    // E(mdb_cursor_open(txn, dbi, &cur2));
    // for (i=0; i<50; i++) {
    //     if (RES(MDB_NOTFOUND, mdb_cursor_get(cur2, &key, &data, MDB_NEXT)))
    //         break;
    //     printf("key: %p %.*s, data: %p %.*s\n",
    //         key.mv_data,  (int) key.mv_size,  (char *) key.mv_data,
    //         data.mv_data, (int) data.mv_size, (char *) data.mv_data);
    //     E(mdb_del(txn, dbi, &key, NULL));
    // }
    //
    // printf("Restarting cursor in txn\n");
    // for (op=MDB_FIRST, i=0; i<=32; op=MDB_NEXT, i++) {
    //     if (RES(MDB_NOTFOUND, mdb_cursor_get(cur2, &key, &data, op)))
    //         break;
    //     printf("key: %p %.*s, data: %p %.*s\n",
    //         key.mv_data,  (int) key.mv_size,  (char *) key.mv_data,
    //         data.mv_data, (int) data.mv_size, (char *) data.mv_data);
    // }
    // mdb_cursor_close(cur2);
    // E(mdb_txn_commit(txn));
    //
    // printf("Restarting cursor outside txn\n");
    // E(mdb_txn_begin(env, NULL, 0, &txn));
    // E(mdb_cursor_open(txn, dbi, &cursor));
    // for (op=MDB_FIRST, i=0; i<=32; op=MDB_NEXT, i++) {
    //     if (RES(MDB_NOTFOUND, mdb_cursor_get(cursor, &key, &data, op)))
    //         break;
    //     printf("key: %p %.*s, data: %p %.*s\n",
    //         key.mv_data,  (int) key.mv_size,  (char *) key.mv_data,
    //         data.mv_data, (int) data.mv_size, (char *) data.mv_data);
    // }
    // mdb_cursor_close(cursor);
    // mdb_txn_abort(txn);
    //
    // mdb_dbi_close(env, dbi);
    // mdb_env_close(env);

    try {
        /* this is an echo server that properly passes every supported Autobahn test */
        uWS::Server server(3000);

        server.onConnection([](uWS::Socket socket) {
            cout << "[Connection] clients: " << ++connections << endl;
        });

        server.onMessage([](uWS::Socket socket, const char *message, size_t length, uWS::OpCode opCode) {
            socket.send((char *) message, length, opCode);
        });

        server.onDisconnection([](uWS::Socket socket, int code, char *message, size_t length) {
            cout << "[Disconnection] clients: " << --connections << endl;
        });

        server.run();

    } catch (...) {
        cout << "ERR_LISTEN" << endl;
    }

    return 0;
}
