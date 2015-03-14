import ifaint
from . create import create_arrowhead

class SvgBuildState:
    """Keeps track of what defs have been generated during SVG
    creation."""

    def __init__(self):
        self.linear_gradients = []
        self.radial_gradients = []
        self.patterns = []

        # Maps Faint fill types to their list and id-prefix
        self.class_map = {
            ifaint.LinearGradient: ('lgradient', self.linear_gradients),
            ifaint.Pattern: ('pattern', self.patterns),
            ifaint.RadialGradient: ('rgradient', self.radial_gradients)}

        # Keep track of whether an arrowhead has been added
        # Fixme: Rework
        self.arrow = True

    def set_arrow(self, arrowSetting):
        if arrowSetting != 'none':
            self.arrow = True

    def get_marker_elements(self):
        """Returns a list of marker elements"""
        # Fixme: Currently only arrowhead
        if self.arrow:
            return [create_arrowhead(),]
        return []

    def get_items(self):
        """Returns all def-entries with their id as a list of tuples"""
        combined_list = []
        for prefix, item_list in self.class_map.values():
            combined_list.extend(zip(self._get_id_range(prefix, len(item_list)),
                                 item_list))
        return combined_list

    def get_defs_id(self, item):
        """Returns the id for the specified item in the <defs>-element for
        linking.

        """
        prefix, item_list = self.class_map[item.__class__]
        return self._get_id(item, prefix, item_list)

    def should_link(self, item):
        """True if the passed in item should be linked to in the <defs> or
        used inline.

        """
        return item.__class__ in self.class_map.keys()

    def _get_id(self, item, prefix, item_list):
        """Get the id for the passed in item. Inserts the item and returns a
        new id if not yet managed.

        """
        try:
            index = item_list.index(item)
        except ValueError:
            index = len(item_list)
            item_list.append(item)

        return self._id_for_index(prefix, index)

    def _get_id_range(self, prefix, item_count):
        # Fixme: Remove?
        return [self._id_for_index(prefix, i) for i in range(item_count)]

    @staticmethod
    def _id_for_index(prefix, index):
        """Generates an id for a defs item."""
        return "%s%d" % (prefix, index + 1)
