/*
 * Originally librhash implementation of libacquire's checksum API
 *
 * TODO: Hide `strcmp` use behind a `#ifdef` allowing use of constant-time equality instead
 * */

#ifndef RHASH_EQ_H
#define RHASH_EQ_H

#include <errno.h>

#if defined(HAS_STDBOOL) && !defined(bool)
#include <stdbool.h>
#else
#include "rhasheq_stdbool.h"
#endif

#include <rhash.h>

#include <c89stringutils_string_extras.h>

#define comma ,
#define blank /* nothing */

int hash_file(const char*, enum rhash_ids,
			  enum rhash_print_sum_flags,
			  char *, char *);

#define HASHES_ \
	X(crc32, comma) \
	X(md4, comma) \
	X(md5, comma) \
	X(tiger, comma) \
	X(tth, comma) \
	X(btih, comma) \
	X(ed2k, comma) \
	X(aich, comma) \
	X(whirlpool, comma) \
	X(ripemd160, comma) \
	X(gost94, comma) \
	X(gost94_cryptopro, comma) \
	X(has160, comma) \
	X(gost12_256, comma) \
	X(gost12_512, comma) \
	X(sha224, comma)  \
	X(sha256, comma) \
    X(sha384, comma) \
    X(sha512, comma) \
	X(edonr256, comma) \
	X(edonr512, comma) \
	X(sha3_224, comma) \
	X(sha3_256, comma) \
	X(sha3_384, comma) \
	X(sha3_512, comma) \
	X(crc32c, comma) \
	X(snefru128, comma) \
	X(snefru256, blank)

#define X(name, unused) \
  bool name(const char *, const char *);
HASHES_
#undef X

#ifdef RHASHEQ_IMPLEMENTATION

int hash_file(const char* filepath,
              const enum rhash_ids hash_id,
              const enum rhash_print_sum_flags print_sum_flags,
              char *digest,
              char *gen_hash)
{
    rhash_library_init(); /* initialize static data */
    {
        const int res = rhash_file(hash_id, filepath, (unsigned char *) digest);
        if (res < 0) {
#if defined(_MSC_VER) || defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
            size_t errmsglen = strerrorlen_s(errno) + 1;
            char *errmsg = malloc(errmsglen);
            strerror_s(errmsg, errmsglen, errno);
            fprintf(stderr, "LibRHash error: %s: %s\n", filepath, errmsg);
            free(errmsg);
#else
            fprintf(stderr, "LibRHash error: %s: %s\n", filepath, strerror(errno));
#endif
        } else
            rhash_print_bytes(gen_hash, (const unsigned char *) digest, rhash_get_digest_size(hash_id),
                              print_sum_flags);
        return res;
    }
}

#define HASHES \
	X(crc32, RHASH_CRC32, comma) \
	X(md4, RHASH_MD4, comma) \
	X(md5, RHASH_MD5, comma) \
	X(tiger, RHASH_TIGER, comma) \
	X(tth, RHASH_TTH, comma) \
	X(btih, RHASH_BTIH, comma) \
	X(ed2k, RHASH_ED2K, comma) \
	X(aich, RHASH_AICH, comma) \
	X(whirlpool, RHASH_WHIRLPOOL, comma) \
	X(ripemd160, RHASH_RIPEMD160, comma) \
	X(gost94, RHASH_GOST94, comma) \
	X(gost94_cryptopro, RHASH_GOST94_CRYPTOPRO, comma) \
	X(has160, RHASH_HAS160, comma) \
	X(gost12_256, RHASH_GOST12_256, comma) \
	X(gost12_512, RHASH_GOST12_512, comma) \
	X(sha224, RHASH_SHA224, comma)  \
	X(sha256, RHASH_SHA256, comma) \
    X(sha384, RHASH_SHA384, comma) \
    X(sha512, RHASH_SHA512, comma) \
	X(edonr256, RHASH_EDONR256, comma) \
	X(edonr512, RHASH_EDONR512, comma) \
	X(sha3_224, RHASH_SHA3_224, comma) \
	X(sha3_256, RHASH_SHA3_256, comma) \
	X(sha3_384, RHASH_SHA3_384, comma) \
	X(sha3_512, RHASH_SHA3_512, comma) \
	X(crc32c, RHASH_CRC32C, comma) \
	X(snefru128, RHASH_SNEFRU128, comma) \
	X(snefru256, RHASH_SNEFRU256, blank)

#define X(name, hash, unused) \
  bool name(const char *filename, const char *gold_hash) { \
    char gen_hash[130], digest[64]; \
	return hash_file(filename, hash, RHPR_HEX, digest, gen_hash) < 0 ? false : strcmp(gen_hash, gold_hash) == 0; \
  }
HASHES
#undef X


#endif /* RHASHEQ_IMPLEMENTATION */

#undef comma
#undef blank

#endif /* !RHASH_EQ_H */
