def dequote(line):
    first_quote = line.find('"')
    second_quote = line.rfind('"')
    return line[first_quote + 1:second_quote]


def xmlify(txt):
    real_quotes = txt.replace('\\"', '"')
    xml_newlines = real_quotes.replace('\\n', '&#10;')
    return xml_newlines


names = []
titles = []
descriptions = []
events = []
levels = []

STATE_BEGIN = 0
STATE_NAMES = 1
STATE_DESC = 2
STATE_EVENTS = 3
STATE_LEVELS = 4
state = STATE_BEGIN

with open('C:/Users/Michael Kristofik/Documents/dev/project-ironfist/src/cpp/shared/artifacts.cpp') as f:
    for line in f:
        if state == STATE_BEGIN and 'gArtifactNames' in line:
            state = STATE_NAMES
            continue
        elif state == STATE_NAMES:
            if '"' in line:
                names.append(dequote(line))
            elif 'gArtifactDesc' in line:
                state = STATE_DESC
                next(f)
                continue
        elif state == STATE_DESC:
            if '"' in line:
                raw_desc = dequote(line)
                fields = raw_desc.split('\\n\\n')
                titles.append(xmlify(fields[0][1:-1]))
                descriptions.append(fields[1])
            elif 'gArtifactEvents' in line:
                state = STATE_EVENTS
                next(f)
                continue
        elif state == STATE_EVENTS:
            if '"' in line:
                events.append(xmlify(dequote(line)))
            elif 'gArtifactLevel' in line:
                state = STATE_LEVELS
                next(f)
                continue
        elif state == STATE_LEVELS:
            if '16' in line:
                levels.append('spellbook')
            elif '32' in line:
                levels.append('unused')
            elif '1' in line or '254' in line:
                levels.append('ultimate')
            elif '2' in line:
                levels.append('major')
            elif '4' in line:
                levels.append('minor')
            elif '8' in line:
                levels.append('treasure')
            else:
                break

print('<?xml version="1.0"?>')
print('<artifacts xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"')
print('           xsi:noNamespaceSchemaLocation="artifacts_xml.xsd">')

for i, name in enumerate(names):
    print('    <artifact id="{0:d}" name="{1:s}" level="{2:s}">'.format(i, name, levels[i]))
    print('        <title>{0:s}</title>'.format(titles[i]))
    print('        <description>{0:s}</description>'.format(descriptions[i]))
    if events[i]:
        print('        <event>{0:s}</event>'.format(events[i]))
    print('    </artifact>')

print('</artifacts>')
