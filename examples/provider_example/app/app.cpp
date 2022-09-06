/**
 * @file app.cpp
 * @brief
 *
 * Code this file just like Arduino way
 *
 * 就像使用Arduino框架一样在这个文件中写代码就可以了
 *
 */

#include "app.h"

void App::setup()
{

    // callback for provider
    // you could return information or null pointer
    // provider 的回调函数
    // 你可以返回信息或空指针
    auto fnCallback = [](
                          // arguments 参数
                          ProviderArguments arguments) -> Element *
    {
        // do your things here with arguments
        // you could convert every argument to supported types
        // in web page, js file
        // or use string directly
        // 你可以在这用参数执行你的操作
        // 你可以提前在前端把参数转换为支持的类型
        // 或者直接使用字符串
        if (arguments->size())
        {
            auto argument0 = arguments->at(0);

            if (argument0->available())
            {
                // ...
            }
        }

        // return information, no need to delete object
        // 返回信息, 不需要手动释放对象
        return new Element("Hello world!");
        // return new Element(0x10);

        /*
         * char buf[24] = {0};
         * sprintf(buf, "Time: %llu", globalTime->getTime() );
         * return new Element(buf);
         */

        // ...

        // or return null pointer
        // 或者返回空指针
        // return nullptr;
    };

    // create new provider
    // this provider only belongs to this app
    // 创建新的provider
    // 这个provider只属于当前app
    global->createProvider(

        // callback 回调函数
        fnCallback,

        // provider name, provider 名称
        "Provider 0",

        // provider settings, provider 选项
        (PROVIDER_ADMIN | PROVIDER_COMMON),

        // how many arguments this provider needs, for web page use, default is 0
        // 这个provider 需要多少个参数, 给前端用的, 默认为0
        0,

        // custom id, for other purpose, optional
        // 其他用途的自定义id，可省略
        12345ULL);

    /*
        if you added "PROVIDER_USER" in settings
        this provider will show it to any users
        this is especially for web page js using
        if you added "PROVIDER_QUESTION" in settings
        means this provider should question user
        to confirm action
        though anyone could send request to
        execute those providers only
        administrator can execute
        don't worry, the request will be confirmed
        at local, provider will NOT be executed
        if user role isn't administrator

        如果你在选项中添加了 "PROVIDER_USER"
        那么这个 provider 会显示给所有用户
        这个是为了前端用的
        如果你在选项添加了 "PROVIDER_QUESTION"
        代表这个操作需要确认，典型的，比如双击或者
        弹窗询问
        尽管所有人都可以通过直接发送请求
        比如直接使用控制台企图用普通用户执行
        管理员才能执行的provider
        但是无须担心
        请求会在本地被确认
        如果用户角色不是管理员
        则请求会被丢弃
    */
    global->createProvider(
        [](ProviderArguments arguments) -> Element *
        {
            // ...
            return new Element("OK");
        },
        "Provider 1",
        (PROVIDER_USER | PROVIDER_QUESTION),
        2);

    /*
        if you added "PROVIDER_ENCRYPT" in settings
        both arguments and return value will be encrypted
        with AES-256-CBC before transfer to target
        I think this is safer than https

        如果你在选项添加了 "PROVIDER_ENCRYPT"
        那么参数和返回值都会使用 AES-256-CBC 加密后
        再传输给对方
        我认为这比https更安全
    */
    global->createProvider(
        [](ProviderArguments arguments) -> Element *
        {
            // arguments had been decrypted before callback
            // so just use it is fine
            // 参数在回调函数执行前已经被解密
            // 直接使用就可以了
            // ...
            return new Element("OK");
        },
        "Provider 2",
        (PROVIDER_ADMIN | PROVIDER_COMMON | PROVIDER_ENCRYPT),
        2);

    global->createProvider(
        [](ProviderArguments arguments) -> Element *
        {
            // ...
            return new Element("OK");
        },
        "Provider 3",
        PROVIDER_USER,
        0,

        // custom id could use as many purpose
        // like execute provider by http get method
        // other purposes depend on your design
        // 自定义 id 可以用作许多用途
        // 比如通过 http get 方法执行provider
        // 其他用途取决于你的设计
        8888ULL);
}

void App::loop()
{
    // put your code in this function
    // it will run repeatedly, attention attached

    // 把你的代码放在这个函数里，代码会重复的运行，请阅读注意事项
}

/**
 * @brief App instance defined here
 *
 * 类App实例在此实例化
 *
 */
App *app = new App();