;; Variable definitions for the paths used in the various faint-elisp
;; files.

(defcustom faint-root
  "" ; No default
  "Faint root folder"
  :type 'file)

(defun faint-defs-file-name ()
  (concat (file-name-as-directory faint-root)
          "build-sys/code_utils/defs-summary.txt"))


(defun faint-build-dir ()
  (concat (file-name-as-directory faint-root)
          (file-name-as-directory "build")))


(defun faint-build-dir ()
  (concat (file-name-as-directory faint-root)
          (file-name-as-directory "build")))


(defun faint-code-utils-dir ()
  (concat (file-name-as-directory faint-root)
          (file-name-as-directory "build-sys")
          (file-name-as-directory "code_utils")))

(defun faint-wx-doc ()
  (interactive)
  (browse-url "http://docs.wxwidgets.org/trunk/"))

(defun faint-wx-search (term)
  (interactive "sSearch term: ")
  (browse-url (concat "http://docs.wxwidgets.org/trunk/search.php?query=" term)))
