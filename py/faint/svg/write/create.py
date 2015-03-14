import xml.etree.ElementTree as ET

def create_arrowhead():
    """Creates a forward-pointing arrow-head marker."""
    element = ET.Element('marker')
    element.set('id', 'Arrowhead')
    element.set('markerUnits', 'strokeWidth')
    element.set('markerWidth', '7.5')
    element.set('markerHeight', '6.6')
    element.set('orient', 'auto')
    element.set('refX', '0')
    element.set('refY', '3.3') # Offset by half width

    path = ET.Element('path')
    path.set('d', "M 0 0 L 7.5 3.3 L 0 6.6 z")
    element.append(path)
    return element
