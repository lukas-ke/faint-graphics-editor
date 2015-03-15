from inspect import getsourcelines as gsl
import inspect

def error(script, message):
    with open("errors.log", 'a') as f:
        f.write('%s: %s\n' % (script, message))

def error_if(condition, script, message):
    if condition:
        error(script, message)

class Logger:
    def __init__(self, script):
        self.script = script

    def error(self, message):
        error(self.script, message)

    def error_if(self, condition, message):
        error_if(condition, self.script, message)

    def Assert(self, condition):
        if not condition:
            frame, filename, line_no, function_name, lines, index = inspect.stack()[1]
            if function_name is None:
                function_name = ""
            if lines is None:
                lines = []
            error(self.script, "Assert in %s: % s\n" % (function_name, "\n".join([l.strip() for l in lines])))
