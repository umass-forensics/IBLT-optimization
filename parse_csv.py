import re

param_file = 'param.export.0.995833.2018-07-16.csv'
template_file = 'iblt_params.h.tpl' 
h_file = 'iblt_params.h' 

with open(param_file) as fd:
    rows = [row.rstrip('\n').split(',') for row in fd.readlines()]
    header = rows.pop(0)
    print(header)
    items = []
    for row in rows:
        n_items = int(row[header.index('items')])
        hedge = float(row[header.index('hedge')])
        numhashes = int(row[header.index('keys')])

        items.append('\t{%d, IbltParamItem(%f, %d)},' % (n_items, hedge, numhashes))

with open(template_file) as fd:
    template = fd.read()
    file_contents = re.sub('<<<items>>>', '\n'.join(items), template)
    print(h_file)

with open(h_file, 'w') as fd:
    fd.write(file_contents)
