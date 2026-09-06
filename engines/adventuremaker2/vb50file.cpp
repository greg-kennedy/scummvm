/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "adventuremaker2/vb50file.h"

#include "common/util.h"
#include "common/debug.h"

namespace AdventureMaker2 {

VB50File::VB50File(Common::SeekableReadStream *s): _s(s) {}

/**
 * Advance stream to next non-whitespace character
 * @return FALSE if error encountered, TRUE otherwise.
 */
bool VB50File::skipWhitespace() {
	char c;
	do {
		c = readSByte();

		if (eos() || err()) {
			// ran out of bytes to read (or something else bad happened)
			debug("error reading from stream in AdventureMaker2::File::skipWhitespace()");
			return false;
		}
	} while (Common::isSpace(c));

	// found a non-space, seek backwards 1 and return
	seek(-1, SEEK_CUR);
	return true;
}

/**
 * Read a String from the stream.
 * In AM2, a string begins with double-quotes, and does not terminate
 *  until the next double-quotes - even if that is on another line!
 * Leading whitespace (space, tab) is skipped.
 * There are no escape characters.  It is impossible to encode a
 *  double-quote in an AM2 string.
 * @param label Optional label for debug print
 * @return String read from stream. On error, empty or partial string returned.
 */
Common::String VB50File::readString(const Common::String label) {
	Common::String result = "";

	// advance past any leading white space
	if (skipWhitespace()) {
		bool inQuotes = false;
		char c;
		do {
			c = readSByte();

			if (eos() || err()) {
				// ran out of bytes to read (or something else bad happened)
				debug("error reading from stream in AdventureMaker2::File::readString()");
				break;
			} else if (c == '"') {
				if (!inQuotes)
					// found the first quotes?
					inQuotes = true;
				else
					// found the final quotes, ready to exit
					break;
			} else {
				if (!inQuotes) {
					// found a character outside "", this is Probably Bad.  give up.
					debug("Non-whitespace character outside quotes in AdventureMaker2::File::readString()");
					break;
				} else if (c != '\r')
					// a non-CR char gets appended.  (this is CRLF -> LF conversion)
					result += c;
			}
		} while (true);
	}

	debug("%s: '%s'", label.c_str(), result.c_str());

	return result;
}

/**
 * Read a Number (floating point) from the stream
 *  a Number is stored without quotes, but trimming works - so just strtod is fine.
 * @param label Optional label for debug print
 * @return Value read from stream. On error, either 0 or the return from strtod.
 */
double VB50File::readNumber(const Common::String label) {
	double result = 0;

	//  Advance stream to first non-space char
	if (skipWhitespace()) {

		Common::String str = readLine();

		if (eos() || err()) {
			// ran out of bytes to read (or something else bad happened)
			debug("error reading from stream in AdventureMaker2::File::readNumber()");
		} else {
			// convert the result to a double (floating-point)
			errno = 0;
			result = strtod(str.c_str(), nullptr);
			if (errno) {
				debug("error converting to double in AdventureMaker2::File::readNumber('%s'): %s", str.c_str(), strerror(errno));
			}
		}
	}

	debug("%s: %lf", label.c_str(), result);
	return result;
}

/**
 * Read a Boolean value from the stream
 *  a Boolean is the exact string #TRUE# or #FALSE#
 *   it does appear the line is trimmed of space and tab, though.
 * @param label Optional label for debug print
 * @return Value read from stream. On error, "false" is returned.
 */
bool VB50File::readBoolean(const Common::String label) {
	bool result = false;

	if (skipWhitespace()) {
		Common::String str = readLine();

		if (eos() || err()) {
			// ran out of bytes to read (or something else bad happened)
			debug("error reading from stream in AdventureMaker2::File::readBoolean()");
		} else {
			// remove trailing whitespace.
			str.trim();

			if (str == "#TRUE#")
				return true;

			else if (str != "#FALSE#") {
				debug("error converting to boolean in AdventureMaker2::File::readBoolean('%s')", str.c_str());
			}
		}
	}

	debug("%s: %s", label.c_str(), result ? "TRUE" : "FALSE");

	return result;
}

} // namespace AdventureMaker2
