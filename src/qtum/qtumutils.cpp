#include <qtum/qtumutils.h>
#include <libdevcore/CommonData.h>
#include <pubkey.h>
#include <util/convert.h>
#include <chainparams.h>

using namespace dev;

bool qtumutils::btc_ecrecover(const dev::h256 &hash, const dev::u256 &v, const dev::h256 &r, const dev::h256 &s, dev::h256 &key)
{
    // Check input parameters
    if(v >= 256)
    {
        // Does not fit into 1 byte
        return false;
    }

    // Convert the data into format usable for btc
    CPubKey pubKey;
    std::vector<unsigned char> vchSig;
    vchSig.push_back((unsigned char)v);
    vchSig += r.asBytes();
    vchSig += s.asBytes();
    uint256 mesage = h256Touint(hash);

    // Recover public key from compact signature (65 bytes)
    // The public key can be compressed (33 bytes) or uncompressed (65 bytes)
    // Pubkeyhash is RIPEMD160 hash of the public key, handled both types
    if(pubKey.RecoverCompact(mesage, vchSig))
    {
        // Get the pubkeyhash
        CKeyID id = pubKey.GetID();
        size_t padding = sizeof(key) - sizeof(id);
        memset(key.data(), 0, padding);
        memcpy(key.data() + padding, id.begin(), sizeof(id));
        return true;
    }

    return false;
}

struct EthChainIdCache
{
    EthChainIdCache() {}
    uint16_t nDefaultPort = 0;
    int beforeShanghaiChainId = 0;
    int afterShanghaiChainId = 0;
};

int qtumutils::eth_getChainId(int blockHeight, int shanghaiHeight, const ChainType& chain)
{
    if (chain == ChainType::MAIN || blockHeight < shanghaiHeight)
        return ChainIdType::MAIN;

    if (chain == ChainType::REGTEST || chain == ChainType::UNITTEST)
        return ChainIdType::REGTEST;

    return ChainIdType::TESTNET;
}

int qtumutils::eth_getChainId(int blockHeight)
{
    const CChainParams& params = Params();
    int shanghaiHeight = params.GetConsensus().nShanghaiHeight;
    uint16_t nDefaultPort = params.GetDefaultPort();
    static EthChainIdCache idCache;
    if(idCache.nDefaultPort != nDefaultPort)
    {
        ChainType chain = params.GetChainType();
        idCache.nDefaultPort = nDefaultPort;
        idCache.beforeShanghaiChainId = eth_getChainId(0, shanghaiHeight, chain);
        idCache.afterShanghaiChainId = eth_getChainId(shanghaiHeight, shanghaiHeight, chain);
    }

    return blockHeight < shanghaiHeight ? idCache.beforeShanghaiChainId : idCache.afterShanghaiChainId;
}
