# Development guide

The app is structured around Widgets/Modules communicating with Presenters
communicating with Repositories. At the top level (see `main.cpp`) you have
`TabsWidget`, which is just a tab layout widget, with `TabsPresenter`, which
is managing tab creation and destruction.

So far there are only two types of top-level tabs/modules defined: Brain List
and Brain Details. Both have corresponding Widgets in `widgets` directory and
Presenters in `presenters` directory.

 ┌─────────────┐                 shows in tabs
 │ TabsWidget  ├──────────────────────────────────────────┐
 └─────┬───────┘                                          │
			 │communicates with                                 │
 ┌─────┴───────┐                                          │
 │TabsPresenter│                  ┌────────────┐          │
 └─────┬───────┘                 ┌┤BrainsModule├──────────┤
  		 │uses                     │└────────────┘          │
 ┌─────┴───────────────┐ creates │                        │
 │DatabaseModuleFactory├─────────┤                        │
 └─────────────────────┘         │                        │
                                 │┌───────────┐*          │
                                 └┤BrainModule├───────────┘
                                  └───────────┘

## Modules

These modules share the structure with any other complex widget in the app.
They consist of:

- A Widget class, which defines the UI.
- A Presenter class, which handles signals from the widget; and sometimes
from other sources.
- A Repository class, which provides and manipulates the data.

 ┌─────────────┐ sends input >   ┌────────────┐   < data >   ┌────────────┐
 │   Widget    ├─────────────────┤ Presenter  ├──────────────┤ Repository │
 └─────┬───────┘ < sends updates └─────┬──────┘              └────────────┘
       │                               │
       │                               │
 ┌─────┴───────┐*              ┌───────┴────────┐*
 │Child widgets│               │Other presenters│
 └─────┬───────┘               └────────────────┘
       │
 ┌─────┴───────┐*
 │   .......   │
 └─────────────┘

To get a feel of how it is organized, start with `widgets/brain_list_widget.h`
and `presenters/brain_list_presenter.h`. Any other big component follows that
structure more or less.

## Data storage

Brain catalogue is organized as a simple config folder. So, on linux you'd
have `~/.config/brainlet/brains` folder with a separate directory for each
brain.

Each brain constists of 3 main parts:

- `brain.sqlite` DB file that tracks nodes and connections inside the Brain
- `documents` folder that hold the text context of each node
- `assets` folder that contains binary files that you've added in the text
of any node through `insert image/asset` menu option.

The brain list is managed by `FolderBrainsRepository`, and each brain is
managed by `DatabaseBrainRepository`.

## Brain editor

The editor consists of two main sections: the connections list and the
thought editor.

- Connection list is managed by `CanvasPresenter`
- The thought/text editor is managed by `TextEditorPresenter`

Both a quite complex, containing multiple other components, so take your time
to get familiar with them. Also, both can affect earch other, for example,
you can rename a thought from the canvas and it will reflect in the editor;
and you can add a connection from the editor, and it will update the canvas.

Th canvas widget works with a Layout abstraction, which I initially planned as
an abstraction over how exactly the individual nodes/thoughts are arranged on
the screen. But right now it only has a single `DefaultLayout` implementation.
Look into `layout/` folder for the details.

In terms of model, each node is represented by a `Thought` class described in
`model/thought.h`.

## Text editor

By far the most complex part of the app.

#### Text model

The text model resides in `model/new_text_model.h`. The model breaks down a
markdown file into individual paragraphs or blocks, which can have different
styles (code block, numbered list, image, etc.). That data is later used by
`MarkdownBlock` widgets to properly format the text.

#### UX logic

- `TextEditorPresenter` manages the sync between the widgets and the data
store.
- `MarkdownEditWidget` handles all of the input and "high"-level text layout
and rendering. It manages global editor state, like cursor position, undo
stack, text selection, etc.
- `MarkdownBlock` displays individual paragraphs of the text in the text
editor, including cursor tracker and text selection.
- `MarkdownScrollWidget` widget catches cursor moves and tries to always keep
it visible in the text area.
- `MarkdownConnectionWidget` is a block after the text that renders links from
current thought/node to all other connected nodes.

#### Text editing logic - folding

I decided to go for manual text editing model because I wanted to make fancy
fold/unfold mechanic when the cursor enters and leaves a paragraph. Thus a
LOT of code was written to handle all of the layout and all of the input and
outout, including standard actions like copy, paste, undo, redo, arrow
movement, etc.

`MarkdownEditWidget` is massive, although the logic itself is not that
complicated. Beware that there are probably still some crashes/bugs that I
wasn't able to reproduce or catch in this behemoth.

#### Metadata parsing

There's a bit of pre and post-processing involved in loading a file for a
a node. The repository handles adding and stripping "metadata" from each file
before giving it to the presenter, or saving it to the cold storage. Look into
`entity/database_brain_repository.cpp` for details.

# Testing

There's a list of test files you can find in `tests` folder. Those are not
unit tests, they're more like mini-apps to test individual widgets. I used
them extensively during initial development phase, but now I mostly just test
with the final app build. Still, if you want to isolate some small piece of UI
or logic and test it, feel free to create a new one there.

