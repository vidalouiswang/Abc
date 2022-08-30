#include "provider.h"

Provider::Provider(uint16_t id,
                   ProviderCallback cb,
                   String name,
                   uint8_t settings,
                   uint8_t lengthOfArguments) : id(id), cb(cb), name(name), settings(settings)
{
    if (settings & PROVIDER_ENCRYPT)
    {
        this->encrypt = true;
    }
    // length of arguments will took 3 bits, 8 arguments maximum
    this->settings |= (lengthOfArguments & (uint8_t)0b00000111);
}

uint8_t *Provider::getBuffer(uint64_t *outLen)
{
    std::vector<Element *> container;
    container.push_back(new Element(this->id));
    container.push_back(new Element(this->settings));
    container.push_back(new Element(this->name));
    container.push_back(new Element(this->customID));

    uint8_t *buffer = ArrayBuffer::createArrayBuffer(&container, outLen);

    for (uint32_t i = 0; i < container.size(); ++i)
    {
        delete container.at(i);
    }

    return buffer;
}