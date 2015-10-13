#include "hale_document.h"
#include "hale_stream.h"
#include "hale_encoding.h"

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
document_load(Document *document, ch *path)
{
    File f;
    if (open(&f, path, File::Read))
    {
        u8  b_in[4096];
        u8 *in  = b_in;
        u8 *in_;

        u16 b_out[4096];
        u16 *out  = b_out;
        u16 *out_ = out + hale_array_count(b_out);

        in_ = b_in + read(&f, b_in, hale_array_count(b_in));

//        if (!equal(b_in, b_in + 3, (const u8*)"\xEF\xBB\xBF")) {
//            hale_panic("Unsupported encoding. As a lazy programmer I only support UTF-8. Thank you.");
//        }

        DocumentEdit edit;
        document_edit(&edit, document, NULL);

        CodecState s = {};
        CodecReturn r;

        for (;;)
        {
            r = codec<Encoding::UTF8, Encoding::Hale>(&in, in_, &out, out_, &s);

            if (in == in_) {
                in  = b_in;
                in_ = in + read(&f, b_in, hale_array_count(b_in));
                if (in_ == in) {
                    break;
                }
            }

            if (r == CodecReturn::OutputUsed)
            {
                document_append(&edit, (ch*)b_out, out - b_out);
                out = b_out;
            }
        }
        close(&f);

        return 1;
    }

    return 0;
}

err
document_save(Document *document, ch *path)
{
    hale_unused(document);
    hale_unused(path);
    hale_not_implemented;
    // Save the document to a file at given path.
    // Update the document->path.-
    // Notify views about path change?
    // Update document's grammar.
    // Notify document views about save.
    // Fire event `saved`.
    return E_NOT_IMPLEMENTED;
}

} // namespace hale
