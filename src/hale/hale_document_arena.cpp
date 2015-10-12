#include "hale_document.h"
#include "hale_stream.h"

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
//    File f;
//    if (open(&f, path, File::Read))
//    {
//        DocumentEdit edit;
//        document_edit(&edit, document, NULL);

//        ReadText r;
//        while (read(&f, &r)) {
//            document_append(&edit, r.text, r.length);
//        }
//        close(&s);

//        return 1;
//    }

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
