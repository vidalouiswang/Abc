### English

A key-value typed RAM database for access data in different types conveniently.
There are full comments in header file, check mydb.h for more information.

The file /lib/mydb/mydb.h and mydb.cpp declared and defined 3 instances of class MyDB, they are:

    - db
    - dbUser
    - dbApp

The first one is main database for system use, you app could use this database to store data, but this is not recommended.

The second one stored other users information, you **SHOULD NOT** modify any content by your way in this database.

The last one is for user app, you could do any action to this instance.

### 中文

一个键值对型内存数据库，方便访问不同种类的数据。
头文件中有完整的注释，更多信息请参阅mydb.h。

文件 /lib/mydb/mydb.h 和 mydb.cpp 中声明和定义了3个数据库实例，它们是：

    - db
    - dbUser
    - dbApp

第一个是主数据库，给系统用的，你的app可以使用这个数据库，但是不建议。

第二个存储了其他用户信息，你 **不** 应该对这个数据库用你的方式进行任何操作。

第三个是给你的app用的，你可以随意使用。