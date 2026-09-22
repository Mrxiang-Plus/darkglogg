/*
 * Copyright (C) 2009, 2010 Nicolas Bonnefon and other contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "textdecoder.h"

#include <QStringConverter>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <cerrno>
#include <iconv.h>
#endif

namespace {

// Keep the name spellings in sync with the constants in
// LogData::doSetDisplayEncoding().
struct CodecInfo {
  const char* name;
  int winCodePage;
  const char* iconvName;
  bool qtNative;
};

const CodecInfo kCodecs[] = {
    {"iso-8859-1", 28591, "ISO-8859-1", true},
    {"latin1", 28591, "ISO-8859-1", true},
    {"utf-8", 65001, "UTF-8", true},
    {"utf8", 65001, "UTF-8", true},
    {"utf-16le", 1200, "UTF-16LE", true},
    {"utf-16be", 1201, "UTF-16BE", true},
    {"CP1251", 1251, "CP1251", false},
    {"CP1252", 1252, "CP1252", false},
    {"Big5", 950, "BIG5", false},
    {"GB18030", 54936, "GB18030", false},
    {"Shift-JIS", 932, "SHIFT_JIS", false},
    {"KOI8-R", 20866, "KOI8-R", false},
};

const CodecInfo* findCodec(const QByteArray& name) {
  for (const CodecInfo& codec : kCodecs) {
    if (name.compare(codec.name, Qt::CaseInsensitive) == 0) return &codec;
  }
  return nullptr;
}

// QString::fromUtf16() assumes host endianness, so decode explicitly.
QString fromUtf16(const char* data, int len, bool bigEndian) {
  const int nchars = len / 2;
  QString out;
  out.resize(nchars);
  ushort* dst = reinterpret_cast<ushort*>(out.data());
  for (int i = 0; i < nchars; ++i) {
    const uchar b0 = static_cast<uchar>(data[2 * i]);
    const uchar b1 = static_cast<uchar>(data[2 * i + 1]);
    dst[i] = bigEndian ? static_cast<ushort>((b0 << 8) | b1)
                       : static_cast<ushort>(b0 | (b1 << 8));
  }
  return out;
}

}  // namespace

TextDecoder::TextDecoder(const QByteArray& name, int winCodePage, bool qtNative)
    : name_(name), winCodePage_(winCodePage), qtNative_(qtNative) {}

std::unique_ptr<TextDecoder> TextDecoder::codecForName(const QByteArray& name) {
  const CodecInfo* info = findCodec(name);
  if (!info) {
    // Unknown codec: keep going with Latin-1 rather than dropping text.
    return std::unique_ptr<TextDecoder>(
        new TextDecoder(QByteArrayLiteral("iso-8859-1"), 28591, true));
  }
  return std::unique_ptr<TextDecoder>(
      new TextDecoder(QByteArray(info->name), info->winCodePage, info->qtNative));
}

QString TextDecoder::toUnicode(const QByteArray& data) const {
  return toUnicode(data.constData(), data.size());
}

QString TextDecoder::toUnicode(const char* data, int len) const {
  if (!data || len <= 0) return QString();

  if (qtNative_) {
    const QByteArray lower = name_.toLower();
    if (lower == "utf-8" || lower == "utf8")
      return QString::fromUtf8(data, len);
    if (lower == "utf-16le")
      return fromUtf16(data, len, /*bigEndian=*/false);
    if (lower == "utf-16be")
      return fromUtf16(data, len, /*bigEndian=*/true);
    return QString::fromLatin1(data, len);
  }

#ifdef Q_OS_WIN
  if (winCodePage_ != 0) {
    const int wlen = ::MultiByteToWideChar(static_cast<UINT>(winCodePage_), 0,
                                           data, len, nullptr, 0);
    if (wlen <= 0) return QString::fromLatin1(data, len);
    QString out;
    out.resize(wlen);
    ::MultiByteToWideChar(static_cast<UINT>(winCodePage_), 0, data, len,
                          reinterpret_cast<LPWSTR>(out.data()), wlen);
    return out;
  }
#else
  const CodecInfo* info = findCodec(name_);
  if (info) {
    iconv_t cd = iconv_open("UTF-8", info->iconvName);
    if (cd != reinterpret_cast<iconv_t>(-1)) {
      // Worst case each input byte becomes 3 UTF-8 bytes.
      QByteArray utf8;
      utf8.resize(len * 3);
      char* inbuf = const_cast<char*>(data);
      size_t inbytes = static_cast<size_t>(len);
      char* outbuf = utf8.data();
      size_t outbytes = static_cast<size_t>(utf8.size());
      if (iconv(cd, &inbuf, &inbytes, &outbuf, &outbytes) != static_cast<size_t>(-1)) {
        iconv_close(cd);
        return QString::fromUtf8(utf8.constData(),
                                 utf8.size() - static_cast<int>(outbytes));
      }
      iconv_close(cd);
    }
  }
#endif

  return QString::fromLatin1(data, len);
}
