namespace gui {
enum GuiState {List, Play};

void init();
void tick();
void setState(GuiState newState);
}
