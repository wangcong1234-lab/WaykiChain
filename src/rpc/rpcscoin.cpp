// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2017-2019 The WaykiChain Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcscoin.h"

#include "commons/base58.h"
#include "rpc/core/rpcserver.h"
#include "init.h"
#include "net.h"
#include "miner/miner.h"
#include "util.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#include "tx/dextx.h"

Value submitpricefeedtx(const Array& params, bool fHelp) {
    if (fHelp || params.size() < 2 || params.size() > 3) {
        throw runtime_error(
            "submitpricefeedtx \"$pricefeeds\" \n"
            "\nsubmit a price feed tx.\n"
            "\nthe execution include registercontracttx and callcontracttx.\n"
            "\nArguments:\n"
            "1. \"address\" : Price Feeder's address\n"
            "2. \"pricefeeds\"    (string, required) A json array of pricefeeds\n"
            " [\n"
            "   {\n"
            "      \"coin\": \"WICC|WGRT\", (string, required) The coin type\n"
            "      \"currency\": \"USD|RMB\" (string, required) The currency type\n"
            "      \"price\": (number, required) The price (boosted by 10^) \n"
            "   }\n"
            "       ,...\n"
            " ]\n"
            "3.\"fee\": (numeric, optional) fee pay for miner, default is 10000\n"
            "\nResult pricefeed tx result\n"
            "\nResult:\n"
            "\nExamples:\n"
            + HelpExampleCli("submitpricefeedtx", 'WiZx6rrsBn9sHjwpvdwtMNNX2o31s3DEHH' '[{\"coin\", WICC, \"currency\": \"USD\", \"price\": 0.28}]'\n")
            + "\nAs json rpc call\n"
            + HelpExampleRpc("submitpricefeedtx","\"WiZx6rrsBn9sHjwpvdwtMNNX2o31s3DEHH\" \"[{\"coin\", WICC, \"currency\": \"USD\", \"price\": 0.28}]\"\n"));
    }

    RPCTypeCheck(params, list_of(str_type)(array_type)(int_type);
    EnsureWalletIsUnlocked();

    auto feedUid = CUserID::ParseUserId(params[0].get_str());
    if (!feedUid) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid addr");
    }

    Array arrPricePoints    = params[1].get_array();
    uint64_t fee            = params[2].get_uint64();  // real type

    int validHeight = chainActive.Tip()->nHeight;
    Vector<CPricePoint> pricePoints;
    for (auto objPp : arrPricePoints) {
        const Value& coinValue = find_value(objPp.get_obj(), "coin");
        const Value& currencyValue = find_value(objPp.get_obj(), "currency");
        const Value& priceValue = find_value(objPp.get_obj(), "price");
        if (    coinValue.type() == null_type
            ||  currencyValue.type() == null_type
            ||  priceValue.type() == null_type ) {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "null tpye not allowed!");
        }

        string coinStr = coinValue.get_str();
        CoinType coinType;
        if (coinStr == "WICC") {
            coinType = CoinType::WICC;
        } else if (coinStr == "WUSD") {
            coinType = CoinType::WUSD;
        } else if (coinStr = "WGRT") {
            coinType = CoinType::WGRT;
        } else {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid coin type: %s", coinStr);
        }

        string currencyStr = currencyValue.get_str();
        PriceType currencyType;
        if (currencyStr == "USD") {
            currencyType = PriceType::USD;
        } else if (currencyStr == "CNY") {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "CNY stablecoin not supported yet");
        } else {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid currency type: %s", currencyStr);
        }

        uint64_t price = priceValue.get_int64();
        CCoinPriceType cpt(coinType, currencyType);
        CPricePoint pp(cpt, price);
        pricePoints.push_back(pp);
    }

    CPriceFeedTx tx(feedUid, validHeight, fee, pricepoints);
    return SubmitTx(feedKeyId, tx);
}

Value submitstakefcointx(const Array& params, bool fHelp) {
    if (fHelp || params.size() < 5 || params.size() > 7) {
        throw runtime_error(
            "submitstakefcointx \"addr\" \"coin_type\" \"asset_type\" asset_amount price [fee]\n"
            "\nsubmit a dex buy limit price order tx.\n"
            "\nArguments:\n"
            "1.\"addr\": (string required) order owner address\n"
            "2.\"coin_type\": (string required) coin type to pay\n"
            "3.\"asset_type\": (string required), asset type to buy\n"
            "4.\"asset_amount\": (numeric, required) amount of target asset to buy\n"
            "5.\"price\": (numeric, required) bidding price willing to buy\n"
            "6.\"fee\": (numeric, optional) fee pay for miner, default is 10000\n"
            "\nResult description:\n"
            "\nResult: {tx_hash}\n"
            "\nExamples:\n"
            + HelpExampleCli("submitdexbuylimitordertx", "\"WiZx6rrsBn9sHjwpvdwtMNNX2o31s3DEHH\" \"WUSD\" \"WICC\" 1000000 200000000\n")
            + "\nAs json rpc call\n"
            + HelpExampleRpc("submitdexbuylimitordertx", "\"WiZx6rrsBn9sHjwpvdwtMNNX2o31s3DEHH\" \"WUSD\" \"WICC\" 1000000 200000000\n")
        );
    }

    EnsureWalletIsUnlocked();

    auto pUserId = CUserID::ParseUserId(params[0].get_str());
    if (!pUserId) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid addr");
    }

    CCDPStakeTx tx(txUidIn, uint64_t feesIn, int validHeightIn,
                uint256 cdpTxIdIn, uint64_t bcoinsToStakeIn, uint64_t collateralRatioIn,
                uint64_t scoinsInterestIn);

    return SubmitTx(userKeyId, tx);

}

/*************************************************<< CDP >>**************************************************/
Value submitstakecdptx(const Array& params, bool fHelp) {
    if (fHelp || params.size() < 2 || params.size() > 4) {
        throw runtime_error(
            "submitstakecdptx \"addr\" \"coin_type\" \"asset_type\" asset_amount price [fee]\n"
            "\nsubmit a dex buy limit price order tx.\n"
            "\nArguments:\n"
            "1. \"address\" : CDP staker's address\n"
            "2. \"stake_amount\": (numeric required) WICC coins to stake into the CDP, boosted by 10^8\n"
            "3. \"collateral_ratio\": (numberic required), collateral ratio, boosted by 10^4 times\n"
            "4. \"cdp_id\": (string optional) ID of existing CDP (tx hash of the first CDP Stake Tx)\n"
            "5. \"fee\": (numeric, optional) fee pay for miner, default is 10000\n"
            "\nResult description:\n"
            "\nResult: {tx_hash}\n"
            "\nExamples:\n"
            + HelpExampleCli("submitstakecdptx", "\"WiZx6rrsBn9sHjwpvdwtMNNX2o31s3DEHH\" 20000000000 30000 \"b850d88bf1bed66d43552dd724c18f10355e9b6657baeae262b3c86a983bee71\" 1000000\n")
            + "\nAs json rpc call\n"
            + HelpExampleRpc("submitstakecdptx", "\"WiZx6rrsBn9sHjwpvdwtMNNX2o31s3DEHH\" 2000000000 30000 \"b850d88bf1bed66d43552dd724c18f10355e9b6657baeae262b3c86a983bee71\" 1000000\n")
        );
    }

    EnsureWalletIsUnlocked();

    auto cdpUid = CUserID::ParseUserId(params[0].get_str());
    if (!cdpUid) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid addr");
    }

    CCDPStakeTx tx(cdpUid, feesIn, int validHeightIn,
                uint256 cdpTxIdIn, uint64_t bcoinsToStakeIn, uint64_t collateralRatioIn,
                uint64_t scoinsInterestIn);

    return SubmitTx(userKeyId, tx);
}
Value submitredeemcdptx(const Array& params, bool fHelp) {

}
Value submitliquidatecdptx(const Array& params, bool fHelp) {

}

Value getmedianprice(const Array& params, bool fHelp);
Value listcdps(const Array& params, bool fHelp);
Value listcdpstoliquidate(const Array& params, bool fHelp);

/*************************************************<< DEX >>**************************************************/
Value submitdexbuylimitordertx(const Array& params, bool fHelp) {
    if (fHelp || params.size() < 5 || params.size() > 7) {
        throw runtime_error(
            "submitdexbuylimitordertx \"addr\" \"coin_type\" \"asset_type\" asset_amount price [fee]\n"
            "\nsubmit a dex buy limit price order tx.\n"
            "\nArguments:\n"
            "1.\"addr\": (string required) order owner address\n"
            "2.\"coin_type\": (string required) coin type to pay\n"
            "3.\"asset_type\": (string required), asset type to buy\n"
            "4.\"asset_amount\": (numeric, required) amount of target asset to buy\n"
            "5.\"price\": (numeric, required) bidding price willing to buy\n"
            "6.\"fee\": (numeric, optional) fee pay for miner, default is 10000\n"
            "\nResult detail\n"
            "\nResult:\n"
            "\nExamples:\n"
            + HelpExampleCli("submitdexbuylimitordertx", "\"WiZx6rrsBn9sHjwpvdwtMNNX2o31s3DEHH\" \"WUSD\" \"WICC\" 1000000 200000000\n")
            + "\nAs json rpc call\n"
            + HelpExampleRpc("submitdexbuylimitordertx", "\"WiZx6rrsBn9sHjwpvdwtMNNX2o31s3DEHH\" \"WUSD\" \"WICC\" 1000000 200000000\n")
        );
    }

    EnsureWalletIsUnlocked();

    auto pUserId = CUserID::ParseUserId(params[0].get_str());
    if (!pUserId) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid addr");
    }

    CoinType coinType;
    if (ParseCoinType(params[0].get_str(), coinType)) {
        throw JSONRPCError(RPC_DEX_COIN_TYPE_INVALID, "Invalid coin_type");
    }

    CoinType assetType;
    if (ParseCoinType(params[1].get_str(), assetType)) {
        throw JSONRPCError(RPC_DEX_ASSET_TYPE_INVALID, "Invalid asset_type");
    }

    uint64_t assetAmount = AmountToRawValue(params[2]);
    uint64_t price = AmountToRawValue(params[3]);

    int64_t defaultFee = SysCfg().GetTxFee(); // default fee
    int64_t fee;
    if (params.size() > 4) {
        fee = AmountToRawValue(params[4]);
        if (fee < defaultFee) {
            throw JSONRPCError(RPC_INSUFFICIENT_FEE,
                               strprintf("Given fee(%ld) < Default fee (%ld)", fee, defaultFee));
        }
    } else {
        fee = defaultFee;
    }

    CAccount txAccount;
    if (!pCdMan->pAccountCache->GetAccount(*pUserId, txAccount)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,
                            strprintf("The account not exists! userId=%s", pUserId->ToString()));
    }
    assert(!txAccount.keyId.IsEmpty());

    // TODO: need to support fee coin type
    uint64_t amount = assetAmount;
    if (txAccount.GetFreeBcoins() < amount + fee) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Account does not have enough coins");
    }

    CUserID txUid;
    if (txAccount.RegIDIsMature()) {
        txUid = txAccount.regId;
    } else if(txAccount.pubKey.IsValid()) {
        txUid = txAccount.pubKey;
    } else{
        CPubKey txPubKey;
        if(!pWalletMain->GetPubKey(txAccount.keyId, txPubKey)) {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,
                strprintf("Get pubKey from wallet failed! keyId=%s", txAccount.keyId.ToString()));
        }
        txUid = txPubKey;
    }

    int validHeight = chainActive.Height();
    CDEXBuyLimitOrderTx tx(txUid, validHeight, fee, coinType, assetType, assetAmount, price);

    if (!pWalletMain->Sign(txAccount.keyId, tx.ComputeSignatureHash(), tx.signature))
            throw JSONRPCError(RPC_WALLET_ERROR, "sign tx failed");

    std::tuple<bool, string> ret = pWalletMain->CommitTx(&tx);

    if (!std::get<0>(ret)) {
        throw JSONRPCError(RPC_WALLET_ERROR, std::get<1>(ret));
    }

    Object obj;
    obj.push_back(Pair("hash", std::get<1>(ret)));
    return obj;
}

Value submitdexselllimitordertx(const Array& params, bool fHelp) {
    return Object(); // TODO:...
}

Value submitdexbuymarketordertx(const Array& params, bool fHelp) {
    return Object(); // TODO:...
}

Value submitdexsellmarketordertx(const Array& params, bool fHelp) {
    return Object(); // TODO:...
}

Value submitdexcancelordertx(const Array& params, bool fHelp) {
    if (fHelp || params.size() < 2 || params.size() > 4) {
        throw runtime_error(
            "submitdexcancelordertx \"addr\" \"txid\"\n"
            "\nsubmit a dex cancel order tx.\n"
            "\nArguments:\n"
            "1.\"addr\": (string required) order owner address\n"
            "2.\"txid\": (string required) txid of order tx want to cancel\n"
            "3.\"fee\": (numeric, optional) fee pay for miner, default is 10000\n"
            "\nResult detail\n"
            "\nResult:\n"
            "\nExamples:\n"
            + HelpExampleCli("submitdexcancelordertx", "\"WiZx6rrsBn9sHjwpvdwtMNNX2o31s3DEHH\" "
                             "\"c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8ac\" ")
            + "\nAs json rpc call\n"
            + HelpExampleRpc("submitdexcancelordertx", "\"WiZx6rrsBn9sHjwpvdwtMNNX2o31s3DEHH\" "\
                             "\"c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8ac\"")
        );
    }
    // TODO: ...
    return Object();
}
Value submitdexsettletx(const Array& params, bool fHelp) {
     if (fHelp || params.size() < 2 || params.size() > 4) {
        throw runtime_error(
            "submitdexsettletx \"addr\" \"deal_items\"\n"
            "\nsubmit a dex settle tx.\n"
            "\nArguments:\n"
            "1.\"addr\": (string required) settle owner address\n"
            "2.\"deal_items\": (string required) deal items in json format\n"
            " [\n"
            "   {\n"
            "      \"buy_order_txid\":\"txid\", (string, required) order txid of buyer\n"
            "      \"sell_order_txid\":\"txid\", (string, required) order txid of seller\n"
            "      \"deal_price\":n (numeric, required) deal price "
            "      \"deal_coin_amount\":n (numeric, required) deal amount of coin\n"
            "      \"deal_asset_amount\":n (numeric, required) deal amount of asset\n"
            "   }\n"
            "       ,...\n"
            " ]\n"
            "3.\"fee\": (numeric, optional) fee pay for miner, default is 10000\n"
            "\nResult detail\n"
            "\nResult:\n"
            "\nExamples:\n"
            + HelpExampleCli("submitdexsettletx", "\"WiZx6rrsBn9sHjwpvdwtMNNX2o31s3DEHH\" "
                           "\"[{\\\"buy_order_txid\\\":\\\"c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8ac\\\", "
                           "\\\"sell_order_txid\\\":\\\"c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8a1\\\", "
                           "\\\"deal_price\\\":100000000,"
                           "\\\"deal_coin_amount\\\":100000000,"
                           "\\\"deal_asset_amount\\\":100000000}]\" ")
            + "\nAs json rpc call\n"
            + HelpExampleRpc("submitdexsettletx", "\"WiZx6rrsBn9sHjwpvdwtMNNX2o31s3DEHH\" "\
                           "[{\"buy_order_txid\":\"c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8ac\", "
                           "\"sell_order_txid\":\"c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8a1\", "
                           "\"deal_price\":100000000,"
                           "\"deal_coin_amount\":100000000,"
                           "\"deal_asset_amount\":100000000}]")
        );
    }
    // TODO: ...
    return Object();
}