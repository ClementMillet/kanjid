from PIL import ImageGrab, Image
import os.path
import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, Gio
import pytesseract
import cv2
import numpy as np
from jisho import Client

dir = os.path.dirname(os.path.realpath(__file__))
@Gtk.Template(filename=dir+"/main_window.ui")
class Window1(Gtk.ApplicationWindow):
    __gtype_name__ = "window1"
    
    # Widget from the window
    text_kanji = Gtk.Template.Child()
    text_kana = Gtk.Template.Child()
    text_meaning = Gtk.Template.Child()
    image_kanji = Gtk.Template.Child()
    
    # Maximum size of the displayed image
    max_im_s = 256

    # Jisho api
    client = Client()

    # OCR config
    custom_config = r' -l Japanese --oem 3 --psm 8'
    # (This configuration is optimized for single word)

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        open_action = Gio.SimpleAction(name="open")
        screen_action = Gio.SimpleAction(name="screen")
        open_action.connect("activate", self.open_file_dialog)
        screen_action.connect("activate", self.make_screenshot)
        self.connect("destroy", Gtk.main_quit)
        self.add_action(open_action)
        self.add_action(screen_action)

    def open_file_dialog(self, action, _):
        self._native = Gtk.FileChooserNative(
            title="Open File",
            transient_for=self,
            action=Gtk.FileChooserAction.OPEN,
            accept_label="_Open",
            cancel_label="_Cancel",
            )
        filter = Gtk.FileFilter()
        filter.add_pattern("*.png")
        filter.add_pattern("*.jpg")
        self._native.add_filter(filter)
        self._native.connect("response", self.on_open_response)
        self._native.show()
    
    def on_open_response(self, dialog, response):
        # If the user selected a file...
        if response == Gtk.ResponseType.ACCEPT:
            # ... retrieve the location from the dialog and open it
            self.open_file(dialog.get_file())
        # Release the reference on the file selection dialog now that we
        # do not need it any more
        self._native = None

    def open_file(self, file):
        file.load_contents_async(None, self.open_file_complete)

    def open_file_complete(self, file, result):
        try:
            print('Opening ' + file.peek_path())
            image = Image.open(file.peek_path())
            self.disp_image(image,self.max_im_s)
            print(self.img2string(image))
            self.display_info(self.img2string(image))
            # self.kanji_search(pytesseract.image_to_string(image))
        except Exception as e:
            print(e)

    def make_screenshot(self, action, _):
        img = ImageGrab.grab()
        arr = cv2.cvtColor(np.array(img), cv2.COLOR_RGB2BGR)
        r = cv2.selectROI('select',arr)
        cv2.destroyWindow('select')
        arr = arr[int(r[1]):int(r[1]+r[3]), int(r[0]):int(r[0]+r[2])]
        img = Image.fromarray(arr)
        self.disp_image(img,self.max_im_s)
        print(self.img2string(img))
        self.display_info(self.img2string(img))


    def disp_image(self,img, max_s):
        # resizing the image to fit the app
        width,height = img.size
        if width > height:
            img_r = img.resize(tuple([max_s, int(max_s*height/width)]))
        else:
            img_r = img.resize(tuple([int(max_s*width/height), max_s]))
        # saving the file cause I don't know how to disp an image from main memory to gtk img :/
        img_r.save('image.png')
        self.image_kanji.set_from_file('image.png')
    
    """
    def process_image(self, img):
        thresh, img_bin = cv2.threshold(img, 128, 255, cv2.THRESH_BINARY | cv2.THRESH_OTSU)
        img.show()
    """

    def img2string(self,img):
        res = pytesseract.image_to_string(img,config = self.custom_config)
        return res[0:-1]
    
    def display_info(self,kanji):
        result = self.client.search(kanji)
        info = result.get('data')
        try:
                kanji_j = info[0].get('japanese')[0].get('word')
                kana = info[0].get('japanese')[0].get('reading')
                meaning_tab = info[0].get('senses')[0].get('english_definitions')
                meaning = ''
                for m in meaning_tab:
                    meaning = meaning + m + ', '
                meaning = meaning[0:-2]
                self.text_kanji.set_text(kanji_j)
                self.text_kana.set_text(kana)
                self.text_meaning.set_text(meaning)
        except:
            print('No kanji found !')
            self.text_kanji.set_text('∅')
            self.text_kana.set_text(' ')
            self.text_meaning.set_text('Error : no kanji found')
            
    


window = Window1()
window.show()

Gtk.main()


