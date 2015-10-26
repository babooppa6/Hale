#if HALE_INCLUDES
#include "hale.h"
#include "hale_document.h"
#include "hale_stream.h"
#include "hale_encoding.h"
#endif

namespace hale {

//struct DocumentTextWriter : public TextWriter {
//    DocumentEdit *edit;
//    DocumentTextWriter(DocumentEdit *edit);
//};

//HALE_PLATFORM_TEXT_WRITER(_document_write)
//{
//    DocumentTextWriter *document_writer = (DocumentTextWriter*)(writer);
//    document_append(document_writer->edit, text, length);
//    return 1;
//}

//DocumentTextWriter::DocumentTextWriter(DocumentEdit *edit) :
//    edit(edit)
//{
//    write = _document_write;
//}

err
document_load(Document *document, const ch16 *path)
{
    hale_assert(document);
    hale_assert(path && *path);

    File f;
    if (open(&f, path, File::Read))
    {
        DocumentEdit edit;
        document_edit(&edit, document, NULL);

        ch8  b_in[4096];
        ch8 *in  = b_in;
        ch8 *in_;

        ch b_out[4096 + 1];
        ch *out  = b_out;
        ch *out_ = out + hale_array_count(b_out);

        in_ = in;

        CodecState cs = {};
        CodecReturn cr;

        for (;;)
        {
            if (in == in_)
            {
                in  = b_in;
                in_ = in + read(&f, b_in, hale_array_count(b_in));
                // qDebug() << "Reading" << (in_ - in) << "bytes.";
                if (in_ == in) {
                    if (out - b_out) {
                        // Append the remainder.
                        // qDebug() << "Writing" << (out-b_out) << "codepoints. (remainder)";
                        document_append(&edit, b_out, out - b_out);
                    }
                    break;
                }
            }

            cr = codec<Encoding::UTF8, Encoding::Hale>(&in, in_, &out, out_, &cs);

            if (cr == CodecReturn::OutputUsed)
            {
                // qDebug() << "Writing" << (out-b_out) << "codepoints.";
                document_append(&edit, b_out, out - b_out);
                out = b_out;
            }
        }
        close(&f);

        return 1;
    }

    return 0;
}

err
document_save(Document *document, const ch8 *path)
{
    File f;
    if (open(&f, path, File::Write))
    {
        memi offset = 0;
        memi length = document_length(document);
        memi s;

        // Hm....

        EncodingInfo<Encoding::Hale>::Storage in_buf[4096];
        EncodingInfo<Encoding::UTF8>::Storage out_buf[4096];

        EncodingInfo<Encoding::Hale>::Storage *in  = in_buf;
        EncodingInfo<Encoding::Hale>::Storage *in_ = in;
        EncodingInfo<Encoding::UTF8>::Storage *out = out_buf;
        EncodingInfo<Encoding::UTF8>::Storage *out_ = out_buf + hale_array_count(out_buf);

        // TODO: Write preamble by document->encoding.
        // write(&f, "\xFF\xFE", 2);

        CodecState cs = {};
        cs.input_option = 0x000D000A;
        CodecReturn cr = CodecReturn::OutputUsed;
        while(length != 0)
        {
            if (in == in_)
            {
                s = minimum(hale_array_count(in_buf), length);
                // qDebug() << "Reading" << (s) << "codepoints.";
                document_text(document, offset, s, in_buf, s);
                in = in_buf;
                in_ = in_buf + s;
                offset += s;
                length -= s;
            }

            cr = codec<Encoding::Hale, Encoding::UTF8>(&in, in_, &out, out_, &cs);

            if (cr == CodecReturn::OutputUsed)
            {
                // qDebug() << "Writing" << (out-out_buf) << "codepoints.";
                write(&f, out_buf, (out - out_buf) * sizeof(EncodingInfo<Encoding::UTF8>::Storage));
                out = out_buf;
            }
        }

        if (out - out_buf) {
            // qDebug() << "Writing" << (out-out_buf) << "codepoints. (remainder)";
            write(&f, out_buf, (out - out_buf) * sizeof(EncodingInfo<Encoding::UTF8>::Storage));
        }

        close(&f);

        return 1;
    }

    return 0;
}

} // namespace hale
