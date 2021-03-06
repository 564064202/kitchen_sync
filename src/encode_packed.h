#ifndef ENCODE_PACKED
#define ENCODE_PACKED

#include "message_pack/copy_packed.h"

template <typename DatabaseClient>
string encode(DatabaseClient &client, const Column &column, const PackedValue &value) {
	if (value.is_nil())		return "NULL";
	if (value.is_false())	return "false";
	if (value.is_true())	return "true";

	uint8_t leader = value.leader();

	if ((leader >= MSGPACK_POSITIVE_FIXNUM_MIN && leader <= MSGPACK_POSITIVE_FIXNUM_MAX) ||
		(leader >= MSGPACK_NEGATIVE_FIXNUM_MIN && leader <= MSGPACK_NEGATIVE_FIXNUM_MAX)) {
		return to_string((int)(int8_t)leader); // up-cast to avoid int8_t being interpreted as char
	}

	PackedValueReadStream stream(value);
	Unpacker<PackedValueReadStream> unpacker(stream);

	switch (leader) {
		case MSGPACK_UINT8:
			return to_string((unsigned int)unpacker.template next<uint8_t>()); // up-cast as above

		case MSGPACK_UINT16:
			return to_string(unpacker.template next<uint16_t>());

		case MSGPACK_UINT32:
			return to_string(unpacker.template next<uint32_t>());

		case MSGPACK_UINT64:
			return to_string(unpacker.template next<uint64_t>());

		case MSGPACK_INT8:
			return to_string((int)unpacker.template next<int8_t>()); // up-cast as above

		case MSGPACK_INT16:
			return to_string(unpacker.template next<int16_t>());

		case MSGPACK_INT32:
			return to_string(unpacker.template next<int32_t>());

		case MSGPACK_INT64:
			return to_string(unpacker.template next<int64_t>());

		case MSGPACK_FLOAT: {
			if (sizeof(float) != sizeof(uint32_t)) throw unpacker_error("Can't convert float to/from network byte order on this platform");
			uint32_t copy = ntohl(unpacker.template read_bytes<uint32_t>());
			float result;
			memcpy(&result, &copy, sizeof(result));
			return to_string(result);
		}

		case MSGPACK_DOUBLE: {
			if (sizeof(double) != sizeof(uint64_t)) throw unpacker_error("Can't convert double to/from network byte order on this platform");
			uint64_t copy = ntohll(unpacker.template read_bytes<uint64_t>());
			double result;
			memcpy(&result, &copy, sizeof(result));
			return to_string(result);
		}

		default:
			return "'" + client.escape_column_value(column, unpacker.template next<string>()) + "'";
	}
}

#endif
