#include <cstdlib>
#include <ctime>

#include "hale.h"

#include "hale_document.h"
#include "hale_perf.h"
#include "hale_print.h"

#include "hale_gap_buffer.h"
#include "hale_fixed_gap_buffer.h"

#include "hale_helper_file.h"

#include <tdocumentbuffer.h>
#include <QString>

namespace hale {

hale_internal void
test_string()
{
    Vector<ch> string;
    vector_init(&string);
    vector_insert(&string, 0, (ch*)L"Miblo");
    hale_test(vector_equal(&string, (ch*)L"Miblo"));
    hale_test(vector_equal(&string, (ch*)L"Miblo1") == 0);
    hale_test(vector_equal(&string, (ch*)L"Mibl") == 0);
}

#define GAP_BUFFER_TEST(gap_buffer, test)\
gap_buffer_dump(&gap_buffer, &string);\
qDebug().noquote() << QString((QChar*)vector_ptr(&string), (int)string.count);\
hale_test(vector_equal(&string, test));

hale_internal Vector<r32>
_helper_make_random_sequence(time_t seed, memi count, const char *function)
{
    Vector<r32> random;
    vector_init(&random);

    qDebug() << function << "seed" << seed;
    std::srand(seed);
    for (memi i = 0; i < count; i++) {
        vector_push(&random, r32((r64)std::rand() / (r64)RAND_MAX));
    }

    return random;
}

hale_internal
Vector<ch8>
_helper_arena_text(FixedGapArena *arena)
{
    Vector<ch8> dump;
    vector_init(&dump, arena->size);
    if (arena->size != 0) {
        vector_resize(&dump, arena->size);
        fixed_gap_arena_text(arena, 0, arena->size, vector_data(&dump));
    }
    return dump;
}

hale_internal
void
_helper_arena_print(FixedGapArena *arena)
{
    auto dump = _helper_arena_text(arena);
    print(dump);
    vector_release(&dump);
}

hale_internal
Vector<ch8>
_helper_make_fill(memi count)
{
    Vector<ch8> fill;
    vector_init(&fill, count);
    for (memi i = 0; i < count; i++) {
        vector_push(&fill, (ch8)('a' + i % 26));
    }
    return fill;
}

hale_internal void
test_gap_buffer_stress()
{
    // TODO: Better test realloc in gap_buffer manually.

    Vector<r32> random = _helper_make_random_sequence(std::time(0), 32000, __FUNCTION__);

    memi insert_text_length = 20;
    ch * insert_text = (ch*)L"AllenMiblo0123456789";

    std::basic_string<ch> mirror, buffer;
    r32 r1, r2;
    memi insert_offset;
    memi insert_length;

    GapBuffer gap_buffer;
    gap_buffer_init(&gap_buffer, 10, 5);
    for (memi i = 0; i < vector_count(&random); i += 2)
    {
        r1 = random[i];
        r2 = random[i + 1];
        insert_length = memi((r32)(insert_text_length-1) * r1) + 1;
        hale_assert(insert_length <= insert_text_length && insert_length > 0);
        insert_offset = memi((r32)gap_buffer.length * r2);

        for (memi j = 0; j < 2; j++)
        {
            for (memi k = 0; k < insert_length; k++) {
                gap_buffer_insert(&gap_buffer, insert_offset + k, insert_text + k, 1);
            }
            mirror.insert(mirror.begin() + insert_offset, insert_text, insert_text + insert_length);

            buffer.resize(gap_buffer.length);
            gap_buffer_text(&gap_buffer, 0, gap_buffer.length, &buffer[0]);
            hale_test(mirror == buffer);
        }

    }
    gap_buffer_release(&gap_buffer);
}

hale_internal void
test_buffer_stress_performance()
{
    memi n = 16000;
    Vector<r32> random = _helper_make_random_sequence(std::time(0), n*2, __FUNCTION__);

    memi insert_text_length = 20;
    ch * insert_text = (ch*)L"AllenMiblo0123456789";
    ch8 * insert_text_8 = (ch8*)"AllenMiblo0123456789";

    std::basic_string<ch> mirror, buffer;
    r32 r1, r2;
    memi insert_offset;
    memi insert_length;

    QString qt_string_buffer;
    qt_string_buffer.reserve(1024);


    {   HALE_PERFORMANCE_TIMER(loop);

        memi characters_typed = 0;
        memi words_typed = 0;

        for (memi i = 0; i < vector_count(random); i += 2)
        {
            words_typed++;
            r1 = random[i];
            r2 = random[i + 1];
            insert_length = memi((r32)(insert_text_length-1) * r1) + 1;
            for (memi k = 0; k < insert_length; k++) {
                characters_typed++;
            }
        }

        qDebug() << "Words inserted" << words_typed << "\n"
                 << "Characters typed" << characters_typed << "\n"
                 << "Ratio" << (r64)characters_typed / (r64)words_typed;
    }

    {   HALE_PERFORMANCE_TIMER(qt_string_build);
        for (memi i = 0; i < vector_count(random); i += 2)
        {
            r1 = random[i];
            r2 = random[i + 1];
            insert_length = memi((r32)(insert_text_length-1) * r1) + 1;
            // hale_assert(insert_length <= insert_text_length && insert_length > 0);
            insert_offset = memi((r32)qt_string_buffer.length() * r2);

            for (memi k = 0; k < insert_length; k++) {
                qt_string_buffer.insert((int)insert_offset + k, insert_text[k]);
            }
        }
    }

    std::basic_string<ch> string_buffer;
    string_buffer.reserve(4096);

    {   HALE_PERFORMANCE_TIMER(string_build);
        for (memi i = 0; i < vector_count(random); i += 2)
        {
            r1 = random[i];
            r2 = random[i + 1];
            insert_length = memi((r32)(insert_text_length-1) * r1) + 1;
            // hale_assert(insert_length <= insert_text_length && insert_length > 0);
            insert_offset = memi((r32)string_buffer.length() * r2);

            for (memi k = 0; k < insert_length; k++) {
                string_buffer.insert(string_buffer.begin() + insert_offset + k, 1, insert_text[k]);
            }
        }
    }

    std::basic_string<ch8> string_buffer_8;
    string_buffer_8.reserve(4096);

    {   HALE_PERFORMANCE_TIMER(string_build_8);
        for (memi i = 0; i < vector_count(random); i += 2)
        {
            r1 = random[i];
            r2 = random[i + 1];
            insert_length = memi((r32)(insert_text_length-1) * r1) + 1;
            // hale_assert(insert_length <= insert_text_length && insert_length > 0);
            insert_offset = memi((r32)string_buffer_8.length() * r2);

            for (memi k = 0; k < insert_length; k++) {
                string_buffer_8.insert(string_buffer_8.begin() + insert_offset + k, 1, insert_text_8[k]);
            }
        }
    }

    TDocumentBuffer<ch, int, int> old_gap_buffer(4096, 256);

    {   HALE_PERFORMANCE_TIMER(old_gap_buffer_build);
        for (memi i = 0; i < vector_count(random); i += 2)
        {
            r1 = random[i];
            r2 = random[i + 1];
            insert_length = memi((r32)(insert_text_length-1) * r1) + 1;
            // hale_assert(insert_length <= insert_text_length && insert_length > 0);
            insert_offset = memi((r32)old_gap_buffer.length() * r2);

            for (memi k = 0; k < insert_length; k++) {
                old_gap_buffer.insert(insert_offset + k, 1, insert_text + k);
            }
        }
    }

    GapBuffer gap_buffer2;
    gap_buffer_init(&gap_buffer2, 1024, 256);

    {   HALE_PERFORMANCE_TIMER(gap_buffer_build);
        for (memi i = 0; i < vector_count(random); i += 2)
        {
            r1 = random[i];
            r2 = random[i + 1];
            insert_length = memi((r32)(insert_text_length-1) * r1) + 1;
            // hale_assert(insert_length <= insert_text_length && insert_length > 0);
            insert_offset = memi((r32)gap_buffer2.length * r2);

            for (memi k = 0; k < insert_length; k++) {
                gap_buffer_insert(&gap_buffer2, insert_offset + k, insert_text + k, 1);
            }
        }
    }

    FixedGapArena arena;
    fixed_gap_arena_init(&arena, 1);

    {   HALE_PERFORMANCE_TIMER(fixed_gap_arena_build);
        for (memi i = 0; i < vector_count(random); i += 2)
        {
            r1 = random[i];
            r2 = random[i + 1];
            insert_length = memi((r32)(insert_text_length-1) * r1) + 1;
            // hale_assert(insert_length <= insert_text_length && insert_length > 0);
            insert_offset = memi((r32)arena.size * r2);

            for (memi k = 0; k < insert_length; k++) {
                fixed_gap_arena_insert(&arena, insert_offset + k, (ch8*)(insert_text_8 + k), 1);
            }
        }
    }


    buffer.resize(gap_buffer2.length);
    gap_buffer_text(&gap_buffer2, 0, gap_buffer2.length, &buffer[0]);

    std::basic_string<ch> old_gap_buffer_text;
    old_gap_buffer_text.resize(old_gap_buffer.length());
    old_gap_buffer.getText(0, old_gap_buffer.length(), &old_gap_buffer_text[0], true);

    hale_test(old_gap_buffer_text == buffer);
    hale_test(string_buffer == buffer);
    hale_test(qt_string_buffer.length() == buffer.length());
    memcmp(qt_string_buffer.constData(), buffer.c_str(), buffer.size() * sizeof(ch));

    std::basic_string<ch8> arena_text;
    arena_text.resize(arena.size);
    fixed_gap_arena_text(&arena, 0, arena.size, &arena_text[0]);
    hale_test(string_buffer_8 == arena_text);

    qDebug() << "gap_buffer.length" << gap_buffer2.length;
    qDebug() << "gap_buffer.capacity" << gap_buffer2.capacity;
    qDebug() << "arena.size" << arena.size;
    qDebug() << "arena.capacity" << vector_count(arena.buffers) * buf_capacity;
#ifdef HALE_TEST
    qDebug() << "mem move size " << gap_buffer2.statistics.mem_move_size;
    qDebug() << "mem move count" << gap_buffer2.statistics.mem_move_count;
    qDebug() << "mem move      " << (r64)gap_buffer2.statistics.mem_move_size / (r64)gap_buffer2.statistics.mem_move_count;
#endif

    qDebug() << "old length" << old_gap_buffer.length() * sizeof(ch);
    qDebug() << "old capacity" << old_gap_buffer.capacity() * sizeof(ch);

    gap_buffer_release(&gap_buffer2);
}

hale_internal void
test_document_edit()
{
    Document document;
    document_init(&document);

    DocumentEdit edit;
    document_edit(&edit, &document, 0);
    hale_test(edit.document == &document);
    hale_test(edit.view == 0);
    hale_test(edit.undo == 0);
    hale_test(edit.internal == 1);
    hale_test(edit.type == DocumentEdit::Insert);
    hale_test(edit.block_changed == 0);
    hale_test(edit.blocks_changed_at == 0);
    hale_test(edit.blocks_changed_count == 0);
    hale_test(edit.pos_begin.block == 0);
    hale_test(edit.pos_begin.position == 0);
    hale_test(edit.pos_end.block == 0);
    hale_test(edit.pos_end.position == 0);
    hale_test(edit.offset_begin == 0);
    hale_test(edit.offset_end == 0);
    hale_test(edit.undo_head == 0);
}

hale_internal void
test_document_insert_edit()
{
    Document document;
    document_init(&document);

    DocumentEdit edit;
    document_edit(&edit, &document, 0);

    document_insert(&edit, {0, 0}, "Hello");
    hale_test(edit.type == DocumentEdit::Insert);
    hale_test(edit.block_changed == 0);
    hale_test(edit.blocks_changed_at == 0);
    hale_test(edit.blocks_changed_count == 0);
    hale_test(edit.pos_begin.block == 0);
    hale_test(edit.pos_begin.position == 0);
    hale_test(edit.pos_end.block == 0);
    hale_test(edit.pos_end.position == 5);
    hale_test(edit.offset_begin == 0);
    hale_test(edit.offset_end == 5);

    document_insert(&edit, edit.pos_end, "\n");
    hale_test(edit.type == DocumentEdit::Insert);
    hale_test(edit.block_changed == 0);
    hale_test(edit.blocks_changed_at == 1);
    hale_test(edit.blocks_changed_count == 1);
    hale_test(edit.pos_begin.block == 0);
    hale_test(edit.pos_begin.position == 5);
    hale_test(edit.pos_end.block == 1);
    hale_test(edit.pos_end.position == 0);
    hale_test(edit.offset_begin == 5);
    hale_test(edit.offset_end == 6);

    document_release(&document);
}

hale_internal void
test_document_insert()
{
    Document document;
    document_init(&document);

    DocumentEdit edit;
    document_edit(&edit, &document, 0);

    hale_test(document.blocks[0].end == 0);
    hale_test(vector_count(document.blocks) == 1);

    document_insert(&edit, {0, 0}, "Hello\n");
    hale_test(document.blocks[0].end == 6);
    hale_test(document.blocks[1].end == 6);
    hale_test(vector_count(document.blocks) == 2);

    document_insert(&edit, {1, 0}, "World\n");
    hale_test(document.blocks[0].end == 6);
    hale_test(document.blocks[1].end == 12);
    hale_test(document.blocks[2].end == 12);
    hale_test(vector_count(document.blocks) == 3);

    document_insert(&edit, {2, 0}, "!");
    hale_test(document.blocks[0].end == 6);
    hale_test(document.blocks[1].end == 12);
    hale_test(document.blocks[2].end == 13);
    hale_test(vector_count(document.blocks) == 3);

    document_release(&document);
}

hale_internal void
test_document_load()
{
    Document document;
    document_init(&document);
    document_load(&document, (ch8*)__FILE__);
    Vector<ch> document_v;
    vector_init(&document_v, document_length(&document));
    vector_resize(&document_v, document_length(&document));
    document_text(&document, 0, document_length(&document),
                  vector_begin(&document_v),
                  vector_count(&document_v));
    document_release(&document);

    auto r = read_file<Encoding::UTF8, Encoding::Hale>((ch8*)__FILE__);
    hale_assert(r.status == 1);

    File out;
    hale_assert(open(&out, (ch8*)(__PROJECT__ "tests/temp/test_document_load.txt"), File::Write));
    write(&out, "\xFF\xFE", 2);
    write(&out,
          vector_begin(&document_v),
          vector_count(&document_v) * sizeof(ch));
    close(&out);

    hale_assert(vector_count(document_v) == vector_count(r.out));
    for (memi i = 0; i < vector_count(document_v); i++)
    {
        ch16 d = document_v[i];
        ch16 o = r.out[i];
        hale_test(d == o);
    }

    vector_release(&document_v);
    vector_release(&r.out);
}

hale_internal void
test_document_save()
{
    ch8 *expected_path = (ch8*)__FILE__;
    ch8 *actual_path = (ch8*)(__PROJECT__ "tests/temp/test_document_save.txt");
    Document document;
    document_init(&document);
    document_load(&document, expected_path);
    document_save(&document, actual_path);
    document_release(&document);

    auto expected = read_file<Encoding::UTF8, Encoding::Hale>(expected_path);
    auto actual = read_file<Encoding::UTF8, Encoding::Hale>(actual_path);

    hale_assert(expected.status == 1);
    hale_assert(actual.status == 1);

    hale_test(vector_count(&expected.out) == vector_count(&actual.out));
    for (memi i = 0; i < vector_count(expected.out); i++)
    {
        ch16 e = expected.out[i];
        ch16 a = actual.out[i];
        hale_test(e == a);
    }
    // hale_test(equal(&expected.out, &actual.out));

    vector_release(&expected.out);
    vector_release(&actual.out);
}

//
// Fixed gap buffer
//

hale_internal void
test_fixed_gap_buffer_buf_stress()
{
    Vector<ch8> dump;
    vector_init(&dump);

    Buf buf;
    buf_allocate(&buf);

    time_t seed;
#if 0
    seed = std::time(0);
#endif
    seed = 1443842828;
    Vector<r32> random = _helper_make_random_sequence(seed, 10, __FUNCTION__);

    memi text_length = 20;
    const ch8 *text = (const ch8*)"AllenMiblo0123456789";

    memi insert_length,
         insert_offset;

    r32 r1, r2;
    std::basic_string<ch8> mirror, s;
    for (memi i = 0; i < vector_count(random); i += 2)
    {
        r1 = random[i];
        r2 = random[i + 1];
        insert_length = memi((r32)(text_length-1) * r1) + 1;
        insert_offset = memi((r32)buf_length(&buf) * r2);

        for (memi k = 0; k < insert_length; k++)
        {
            buf_insert(&buf, insert_offset + k, text + k, 1);
            mirror.insert(mirror.begin() + insert_offset + k, text[k]);

            s.resize(buf_length(&buf));
            buf_text(&buf, 0, s.length(), &s[0]);
            hale_test(s == mirror);

            if (buf_length(&buf) == 4096) {
                break;
            }
        }
    }

    print(s.c_str());
    print(mirror.c_str());
    buf_dump(&buf, &dump);
    print(&dump);

    buf_deallocate(&buf);
}

hale_internal
void
_helper_test_mirror(FixedGapArena *arena, std::basic_string<ch8> &mirror)
{
    auto dump = _helper_arena_text(arena);
    bool equal = vector_equal(&dump, mirror.c_str(), mirror.length());
    if (!equal) {
        print("/-------------------------------------------\\\n");
        print("---------------------------------------------\n");
        print("---------------------------------------------\n");
        print("Actual:\n");
        print(dump);
        print("\n");
        print("---------------------------------------------\n");
        print("Expected:\n");
        print(mirror.c_str());
        print("\n");
        print("---------------------------------------------\n");
        print("---------------------------------------------\n");
        print("\\-------------------------------------------/\n");
        hale_test(false);
    }
    vector_release(&dump);
}

//
//
//

hale_internal
void
_helper_test_fixed_gap_arena_insert_f(FixedGapArena *arena,
                                      memi offset,
                                      const ch8 *data,
                                      memi size,
                                      std::basic_string<ch8> &mirror)
{
    fixed_gap_arena_insert(arena, offset, data, size);
    mirror.insert(mirror.begin() + offset, data, data + size);
    _helper_test_mirror(arena, mirror);
}

hale_internal
void
_helper_test_fixed_gap_arena_remove_f(FixedGapArena *arena,
                                      memi offset,
                                      memi size,
                                      std::basic_string<ch8> &mirror)
{
    fixed_gap_arena_remove(arena, offset, size);
    mirror.erase(mirror.begin() + offset, mirror.begin() + offset + size);
    _helper_test_mirror(arena, mirror);
}

// Expects `std::string mirror;` to be defined in the test function.
#define _helper_test_fixed_gap_arena_insert(arena, offset, data, size)\
    _helper_test_fixed_gap_arena_insert_f(arena, offset, data, size, mirror)

#define _helper_test_fixed_gap_arena_remove(arena, offset, size)\
    _helper_test_fixed_gap_arena_remove_f(arena, offset, size, mirror)

//
//
//

hale_internal void
test_fixed_gap_buffer_move_suffix()
{
    // buf_move_suffix(Buf *A, memi offset, Buf *B);

    Buf A, B;
    ch8 *A_begin = (ch8*)platform.allocate_paged_memory(buf_capacity);
    ch8 *A_end = A_begin + buf_capacity;
    ch8 *B_begin = (ch8*)platform.allocate_paged_memory(buf_capacity);
    ch8 *B_end = B_begin + buf_capacity;

    std::basic_string<ch8> a, b, t;
    a.resize(buf_capacity);
    b.resize(buf_capacity);

#define RESET_AB\
    A.gap_start = A_begin;\
    A.gap_end = A_end;\
    hale_debug(A.debug_page = A_begin);\
    hale_debug(A.debug_length = 0);\
    B.gap_start = B_begin;\
    B.gap_end = B_end;\
    hale_debug(B.debug_page = B_begin);\
    hale_debug(B.debug_length = 0);\
    for (memi i = 0; i != buf_capacity; i++) {\
        A.gap_start[i] = a[i] = 'a' + (i % 26);\
        B.gap_start[i] = b[i] = '1' + (i % 13);\
    }

    //
    // < gap_start, == 0
    //

    RESET_AB;

    A.gap_start = A_begin + 10;
    hale_debug(A.debug_length = 10);
    buf_move_suffix(&A, 0, &B);
    hale_test(A.gap_start == A_begin);
    hale_test(A.gap_end == A_end);
    hale_test(B.gap_start = B_begin + 10);
    hale_test(B.gap_end = B_end);

    // referential test
    b.replace(0, 10, A_begin, 10);
    hale_test(equal(&b[0], &b[0] + buf_capacity, buf_page_begin(&B), buf_page_end(&B)));

    //
    // < gap_start
    //

    RESET_AB;

    A.gap_start = A_begin + 3;
    A.gap_end   = A_begin + 6;
    hale_debug(A.debug_length = buf_capacity - (A.gap_end - A.gap_start));
    buf_move_suffix(&A, 5, &B);
    hale_test(A.gap_start == A_begin + 5);
    hale_test(A.gap_end == A_end);
    hale_test(B.gap_start = B_begin + (buf_capacity-5)); // test
    hale_test(B.gap_end = B_end);

    // referential test (B)
    {   memi x = (6-3)+5;
        b.replace(0, buf_capacity-x, A_begin + x, buf_capacity-x);
        hale_test(equal(&b[0], &b[0] + buf_capacity, buf_page_begin(&B), buf_page_end(&B)));
    }

    // referential test (A)
    {   memi offset = (6-3)+5;  // A_ptr += buf_gap(A);
        memi size = offset - 6; // s = A_ptr - A->gap_end;
        b = a;
        a.replace(3 /* gap_start */, size, b.c_str() + 6 /* gap_end */, size);
        hale_test(equal(&a[0], &a[0] + buf_capacity, buf_page_begin(&A), buf_page_end(&A)));
    }

    //
    // empty gap (at 3), move 3, == gap_start
    //

    RESET_AB;

    A.gap_start = A_begin + 3;
    A.gap_end   = A_begin + 3;
    hale_debug(A.debug_length = buf_capacity);
    buf_move_suffix(&A, 3, &B);
    hale_test(A.gap_start == A_begin + 3);
    hale_test(A.gap_end == A_end);
    hale_test(B.gap_start = B_begin + (buf_capacity-3));
    hale_test(B.gap_end = B_end);

    // referential test
    b.replace(0, buf_capacity - 3, A_begin + 3, buf_capacity - 3);
    hale_test(equal(&b[0], &b[0] + buf_capacity, buf_page_begin(&B), buf_page_end(&B)));

    //
    // empty gap (at 0), == gap_start
    //

    RESET_AB;

    A.gap_start = A_begin;
    A.gap_end   = A_begin;
    hale_debug(A.debug_length = buf_capacity);
    buf_move_suffix(&A, 0, &B);
    hale_test(A.gap_start == A_begin);
    hale_test(A.gap_end == A_end);
    hale_test(B.gap_start == B_begin);
    hale_test(B.gap_end == B_begin);

    // Testing special case when B is fully filled, gap_start and gap_end
    // should be reset to beginning.
    hale_test(B.gap_start = B_begin);
    hale_test(B.gap_end = B_begin);

    // referential test
    b.replace(0, buf_capacity, A_begin, buf_capacity);
    hale_test(equal(&b[0], &b[0] + buf_capacity, buf_page_begin(&B), buf_page_end(&B)));

    // Same as above, but with A.gap being at 3 (so we're going the other branch)

    RESET_AB;

    A.gap_start = A_begin + 3;
    A.gap_end   = A_begin + 3;
    hale_debug(A.debug_length = buf_capacity);
    buf_move_suffix(&A, 0, &B);
    hale_test(A.gap_start == A_begin);
    hale_test(A.gap_end == A_end);
    hale_test(B.gap_start == B_begin);
    hale_test(B.gap_end == B_begin);

    // referential test
    b.replace(0, buf_capacity, A_begin, buf_capacity);
    hale_test(equal(&b[0], &b[0] + buf_capacity, buf_page_begin(&B), buf_page_end(&B)));

    //
    // suffix at the A + buf_capacity.
    //

    RESET_AB;

    A.gap_start = A_begin;
    A.gap_end   = A_begin;
    hale_debug(A.debug_length = buf_capacity);
    // should assert
    // buf_move_suffix(&A, buf_capacity, &B);

    A.gap_start = A_begin;
    A.gap_end   = A_begin + 5;
    hale_debug(A.debug_length = (buf_capacity - 5));
    // should assert
    // buf_move_suffix(&A, buf_capacity, &B);

#undef RESET_AB

    platform.deallocate_paged_memory(buf_page_begin(&A));
    platform.deallocate_paged_memory(buf_page_begin(&B));
}

//
// One buffer through arena.
//


hale_internal void
test_fixed_gap_arena_one_buf_insert()
{
    std::basic_string<ch8> mirror;

    FixedGapArena arena;
    fixed_gap_arena_init(&arena, 0);
    // initial
    _helper_test_fixed_gap_arena_insert(&arena, 0, (ch8*)"Hello", 5);
    // ==
    _helper_test_fixed_gap_arena_insert(&arena, 5, (ch8*)"Hello", 5);
    // <
    _helper_test_fixed_gap_arena_insert(&arena, 3, (ch8*)"Miblo", 5);
    // >
    _helper_test_fixed_gap_arena_insert(&arena, 9, (ch8*)"Miblo", 5);

    fixed_gap_arena_release(&arena);
}

hale_internal void
test_fixed_gap_arena_one_buf_remove()
{
    std::basic_string<ch8> mirror;
    Buf *buf;

    FixedGapArena arena;
    fixed_gap_arena_init(&arena, 0);

    // Before gap
    _helper_test_fixed_gap_arena_insert(&arena, 0, (ch8*)"HelloMibloHowAreYou", 19);
    // HelloMibloHowAreYou###
    _helper_test_fixed_gap_arena_remove(&arena, 5, 5);
    // HelloHowAreYou###
    buf = &arena.buffers[0];
    hale_test(buf->gap_start == buf_page_begin(buf) + 14);
    hale_test(buf->gap_end   == buf_page_end(buf));


    // Around the gap
    _helper_test_fixed_gap_arena_insert(&arena, 5, (ch8*)"Miblo", 5);
    // HelloMiblo###HowAreYou
    //      [xxxxxxxxx]
    buf = &arena.buffers[0];
    hale_test(buf->gap_start == buf_page_begin(buf) + 10);
    hale_test(buf->gap_end   == buf_page_end(buf) - 9);
    _helper_test_fixed_gap_arena_remove(&arena, 5, 8);
    // Hello###AreYou
    buf = &arena.buffers[0];
    hale_test(buf->gap_start == buf_page_begin(buf) + 5);
    hale_test(buf->gap_end   == buf_page_end(buf) - 6);

    // After the gap.
    _helper_test_fixed_gap_arena_remove(&arena, 8, 3);
    // HelloAre###
    buf = &arena.buffers[0];
    hale_test(buf->gap_start == buf_page_begin(buf) + 8);
    hale_test(buf->gap_end   == buf_page_end(buf));

    // Remove at end
    _helper_test_fixed_gap_arena_remove(&arena, 5, 3);
    // Hello###
    buf = &arena.buffers[0];
    hale_test(buf->gap_start == buf_page_begin(buf) + 5);
    hale_test(buf->gap_end   == buf_page_end(buf));

    // Complete remove
    _helper_test_fixed_gap_arena_remove(&arena, 0, 5);
    // ###
    // Remove won't delete the last buffer.
    hale_test(vector_count(arena.buffers) == 1);

    fixed_gap_arena_release(&arena);
}

//
//
//


hale_internal
void
_helper_arena_insert(memi fill1, memi offset, memi fill2)
{
    std::basic_string<ch8> mirror;

    FixedGapArena arena;
    fixed_gap_arena_init(&arena, 0);

    Vector<ch8> fill = _helper_make_fill(hale_maximum(fill1, fill2));
    if (fill1) {
        _helper_test_fixed_gap_arena_insert(&arena, 0, vector_data(&fill), (fill1));
    }
    _helper_test_fixed_gap_arena_insert(&arena, offset, vector_data(&fill), (fill2));
    vector_release(&fill);

    fixed_gap_arena_release(&arena);
}

hale_internal void
test_fixed_gap_arena_insert()
{
    // crit
    _helper_arena_insert(0, 0, platform.page_size - 1);
    // crit
    _helper_arena_insert(0, 0, platform.page_size);
    // p0, p2
    _helper_arena_insert(0, 0, platform.page_size + 1);
    // p0, p1
    _helper_arena_insert(0, 0, (platform.page_size * 2));
    // p0, p1, p2
    _helper_arena_insert(0, 0, (platform.page_size * 2) + 1);

    // crit, sx, p0, p2
    _helper_arena_insert(10, 5, platform.page_size);
}

hale_internal
void
_helper_arena_remove(memi fill_count, memi offset, memi size)
{
    std::basic_string<ch8> mirror;

    FixedGapArena arena;
    fixed_gap_arena_init(&arena, 0);

    Vector<ch8> fill = _helper_make_fill(fill_count);
    _helper_test_fixed_gap_arena_insert(&arena, 0, vector_data(&fill), fill_count);
    _helper_test_fixed_gap_arena_remove(&arena, offset, size);

    vector_release(&fill);
    fixed_gap_arena_release(&arena);
}


hale_internal void
test_fixed_gap_arena_remove()
{
    // p0, p2
    // ....x|x....
    _helper_arena_remove(buf_capacity * 2, buf_capacity - 1, 2);
    // ...x#|#x...
    _helper_arena_remove(buf_capacity * 2, buf_capacity - 1, 2);

    // p0, p1, p2
    // ....x|xxxxx|x....
    _helper_arena_remove(buf_capacity * 3, buf_capacity - 1, buf_capacity + 1 + 1);

    // p0 (skipped/special case), p1, p2
    // xxxxx|xxxxx|x....
    _helper_arena_remove(buf_capacity * 3, 0, buf_capacity * 2 + 1);

    // p1, p2
    // .....|xxxxx|x....
    _helper_arena_remove(buf_capacity * 3, buf_capacity, buf_capacity + 1);
}

//
//
//

#define FIXED_GAP_STRESS_PERFORMANCE 1

#if FIXED_GAP_STRESS_PERFORMANCE
#define _helper_test_fixed_gap_arena_stress_insert fixed_gap_arena_insert
#define _helper_test_fixed_gap_arena_stress_remove fixed_gap_arena_remove
#else
#define _helper_test_fixed_gap_arena_stress_insert _helper_test_fixed_gap_arena_insert
#define _helper_test_fixed_gap_arena_stress_remove _helper_test_fixed_gap_arena_remove
#endif

hale_internal void
test_fixed_gap_arena_stress()
{
    // time_t seed = std::time(0);
    time_t seed = 1444138129;
    memi n = 16000;
    memi per_it = 3;
    Vector<r32> random = _helper_make_random_sequence(seed, n*per_it, __FUNCTION__);

    memi op_text_length = 20;
    ch8 * op_text = (ch8*)"AllenMiblo0123456789";

    memi op_offset;
    memi op_length;

    FixedGapArena arena;
    fixed_gap_arena_init(&arena, 1);
    std::string mirror;

    enum Op
    {
        OneInsert = 0, // Type
        OneRemove, // Delete
        AllInsert, // Paste
        AllRemove  // Cut
    };

    memi op;

    {   HALE_PERFORMANCE_TIMER(test_fixed_gap_arena_stress);
        memi i = 0;
        while (i < vector_count(random))
        {
            op = memi((r32)3 * random[i++]);

            if (arena.size == 0) {
                if (op == OneRemove) {
                    op = OneInsert;
                } else if (op == AllRemove) {
                    op = AllInsert;
                }
            }

            if (op == OneInsert || op == AllInsert)
            {
                op_length = memi((r32)(op_text_length-1) * random[i++]) + 1;
                op_offset = memi((r32)arena.size * random[i++]);
            }
            else // OneRemove || AllRemove
            {
                memi random_length = memi((r32)(op_text_length-1) * random[i++]) + 1;
                op_length = hale_minimum(arena.size, random_length);
                op_offset = memi((r32)(arena.size - op_length) * random[i++]);
            }

            const ch8 *op_name;
            switch (op)
            {
            case OneInsert: {
                op_name = (ch8*)"OneInsert";
                for (memi k = 0; k < op_length; k++) {
                    _helper_test_fixed_gap_arena_stress_insert(&arena, op_offset + k, op_text + k, 1);
                }
            } break;
            case OneRemove: {
                op_name = (ch8*)"OneRemove";
                for (memi k = 0; k < op_length; k++) {
                    _helper_test_fixed_gap_arena_stress_remove(&arena, op_offset, 1);
                }
            } break;
            case AllInsert: {
                op_name = (ch8*)"AllInsert";
                _helper_test_fixed_gap_arena_stress_insert(&arena, op_offset, op_text, op_length);
            } break;
            case AllRemove: {
                op_name = (ch8*)"AllRemove";
                _helper_test_fixed_gap_arena_stress_remove(&arena, op_offset, op_length);
            } break;
            default:
                hale_not_reached;
                break;
            }

#if not(FIXED_GAP_STRESS_PERFORMANCE)
            qDebug() << qSetRealNumberPrecision(3) << ((r64)(i*100) / (r64)(n*per_it)) << "%"
                     << "arena.length" << arena.length
                     << "op" << op_name
                     << "op_offset" << op_offset
                     << "op_length" << op_length;
#endif
        }
    }
}

hale_internal void
test_fixed_gap_arena_stress_data()
{
    // time_t seed = std::time(0);
    memi n = 3000;
    time_t seed = 1444138129;
    Vector<r32> random = _helper_make_random_sequence(seed, n*2, __FUNCTION__);

    memi insert_text_length = 20;
    ch8 * insert_text_8 = (ch8*)"AllenMiblo0123456789";

    r32 r1, r2;
    memi insert_offset;
    memi insert_length;

    FixedGapArena arena;
    fixed_gap_arena_init(&arena, 1);
    std::string mirror;

    Vector<r64> data;
    memi buffer_count;
    vector_init(&data, vector_count(random)/2);
    {
        for (memi i = 0; i < vector_count(random); i += 2)
        {
            r64 time = platform.read_time_counter();
            r1 = random[i];
            r2 = random[i + 1];
            insert_length = memi((r32)(insert_text_length-1) * r1) + 1;
            // hale_assert(insert_length <= insert_text_length && insert_length > 0);
            insert_offset = memi((r32)arena.size * r2);

            buffer_count = vector_count(arena.buffers);
            for (memi k = 0; k < insert_length; k++) {
                fixed_gap_arena_insert(&arena,
                                       insert_offset + k,
                                       insert_text_8 + k,
                                       1);
            }
            qDebug().nospace() << (platform.read_time_counter() - time) * 1e6 << "\t" << arena.size << "\t" << vector_count(arena.buffers) - buffer_count;
        }
    }
}

//
//
//

hale_internal void
_helper_test_equal(FixedGapArena *arena, Vector<ch8> *fill, memi begin, memi end)
{
    hale_assert(end >= begin);

    Vector<ch8> tmp;
    vector_init(&tmp);
    vector_resize(&tmp, end-begin + 1); // +1 for testing empty.

    fixed_gap_arena_text(arena, begin, end - begin, vector_begin(&tmp));
    hale_test(equal(vector_begin(fill) + begin,
                    vector_begin(fill) + end,
                    vector_begin(&tmp),
                    vector_begin(&tmp) + (end-begin)
                    ));

    vector_release(&tmp);
}

hale_internal void
test_fixed_gap_text()
{
    FixedGapArena arena;
    fixed_gap_arena_init(&arena, 1);

    Vector<ch8> fill = _helper_make_fill(buf_capacity * 3);
    fixed_gap_arena_insert(&arena,
                           0,
                           vector_data(&fill),
                           vector_count(&fill));

    // empty
    _helper_test_equal(&arena, &fill,
                       0, 0);
    // [**...] [.....] [.....]
    _helper_test_equal(&arena, &fill,
                       0, 1);
    // [*****] [.....] [.....]
    _helper_test_equal(&arena, &fill,
                       0, buf_capacity);
    // [....*] [*....] [.....]
    _helper_test_equal(&arena, &fill,
                       buf_capacity - 1, buf_capacity + 1);
    // [.....] [*....] [.....]
    _helper_test_equal(&arena, &fill,
                       buf_capacity, buf_capacity + 1);
    // [....*] [*****] [*....]
    _helper_test_equal(&arena, &fill,
                       buf_capacity - 1, (buf_capacity*2) + 1);
    // [.....] [....*] [*....]
    _helper_test_equal(&arena, &fill,
                       (buf_capacity*2) - 1, (buf_capacity*2) + 1);
    // [.....] [.....] [*....]
    _helper_test_equal(&arena, &fill,
                       (buf_capacity*2), (buf_capacity*2) + 1);
    // [.....] [.....] [....*]
    _helper_test_equal(&arena, &fill,
                       (buf_capacity*3)-1, (buf_capacity*3));
    // [.....] [.....] [.....* (empty at end)
    _helper_test_equal(&arena, &fill,
                       (buf_capacity*3), (buf_capacity*3));

    vector_release(&fill);
}

//
//
//

hale_internal void
test_fixed_gap_buffer()
{
    test_fixed_gap_text();
    test_fixed_gap_buffer_move_suffix();

    // Buffer operations through Arena.
    test_fixed_gap_arena_one_buf_insert();
    test_fixed_gap_arena_one_buf_remove();

    // Arena tests.
    test_fixed_gap_arena_insert();
    test_fixed_gap_arena_remove();

    // Stress tests.
    test_fixed_gap_buffer_buf_stress();
    test_fixed_gap_arena_stress();
}

void
test_document()
{
    test_document_insert_edit();
    test_document_insert();
    test_document_load();
    test_document_save();
//    test_string();
//    test_fixed_gap_buffer();
}

} // namespace hale
