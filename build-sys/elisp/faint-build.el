;; Build-related functions for developing Faint in GNU Emacs.
;;
;; * faint-build: builds Faint
;; * faint-clean: cleans all Faint build output
;; * faint-fixme: finds fixmes in Faint sources

(require 'compile)
(load-library "faint-paths.el")

(defun faint-build ()
  (interactive)
  (when (= 0 (length faint-root))
    (user-error "faint-root not customized"))
  (compile (concat (faint-build-dir) "build.py --stdout")))

(defun faint-clean ()
  "Clean all build output"
  (interactive)
  (compile (concat (faint-build-dir) "clean.py all")))

(defun faint-fixme ()
  "Finds fixmes in source files, produces the list as
faux-compilation output"
  (interactive)
  (compile (concat (faint-code-utils-dir)
                   "find_fixme.py " faint-root " --emacs")))


(global-set-key [(f5)] 'faint-build)
(global-set-key [(control +)] 'next-error)
(global-set-key [(meta +)] 'previous-error)
(global-set-key [(control \?)] 'compilation-next-file)

(defun faint-run-unit-test ()
  (interactive)
  (let ((default-directory
          (concat (file-name-as-directory faint-root) "/tests/")))
    (shell-command
     (concat (file-name-as-directory faint-root) "tests/run-unit-tests.exe&")
     "*test-results*")))

(defun faint-run-bench ()
  (interactive)
  (let ((default-directory
          (concat (file-name-as-directory faint-root) "/tests/")))
    (shell-command
   (concat (file-name-as-directory faint-root) "tests/run-benchmarks.exe&")
   "*benchmark-results*")))
