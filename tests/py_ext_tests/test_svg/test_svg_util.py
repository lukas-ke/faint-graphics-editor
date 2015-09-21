import unittest
import re # Fixme: Consider compiling regexes instead
import faint.svg.svg_re as svg_re
import faint.svg.parse.parse_state as parse_state
from faint.svg.parse.parse_util import Matrix # Fixme: Move to matrix.py

class TestSVGUtil(unittest.TestCase):

    def test_regex(self):
        self.assertTrue(re.match(svg_re.number_attr, "10.1").group(0) == '10.1')
        self.assertTrue(re.match(svg_re.number_attr, "10").group(0) == '10')
        self.assertTrue(re.match(svg_re.number_attr, "10.10").group(0) == '10.10')
        self.assertTrue(re.match(svg_re.length_attr, "-640.0E10").groups() ==
                        ('-640.0E10',''))
        self.assertTrue(re.match(svg_re.length_attr, "-640pt").groups() ==
                        ('-640','pt'))
        self.assertTrue(re.match(svg_re.length_attr, "640pt").groups() ==
                        ('640','pt'))

    def test_matrix(self):
        def matrix_near(m1, m2):
            # Fixme: Use unittest near check
            d = 0.001
            def near(v1, v2):
                return abs(v1 - v2) < d

            return all(near(v1, v2) for v1, v2 in zip(m1.values(), m2.values()))

        scale, rotate, translate = parse_state.parse_transform_list(
            "scale(3 2) rotate(3) translate(4 112)")

        self.assertTrue(matrix_near(Matrix.scale(3,2), scale))
        self.assertTrue(matrix_near(Matrix.rotation(0.05235), rotate))
        self.assertTrue(matrix_near(Matrix.translation(4, 112), translate))
