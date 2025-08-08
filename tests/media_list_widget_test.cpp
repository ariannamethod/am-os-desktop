#include "../Telegram/SourceFiles/info/media/info_media_common.h"
#include "../Telegram/SourceFiles/info/media/info_media_list_widget.h"
#include <cassert>

int main() {
    using namespace Info::Media;
    ListSelectedMap selected;
    auto item = reinterpret_cast<const HistoryItem*>(1);
    ChangeItemSelection(selected, item, ListItemSelectionData(TextSelection{0,5}));
    assert(selected.size() == 1);
    assert(selected.begin()->second.text.from == 0);
    assert(selected.begin()->second.text.to == 5);
    assert(selected.begin()->second.text != FullSelection);
    int height = 100;
    assert(ComputeDragScrollOffset(-10, height) == -10);
    assert(ComputeDragScrollOffset(110, height) == 10);
    assert(ComputeDragScrollOffset(50, height) == 0);
    return 0;
}
