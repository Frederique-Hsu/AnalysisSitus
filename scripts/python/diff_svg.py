# Install next Python's modules to your Python instance to run this script locally:
#
# * pytest
# * svglib
# * ezdxf
# * opencv-python
# * reportlab
# * matplotlib
# * cairosvg
#
# On windows for 'cairosvg' run next commands:
#
#   pip install pipwin
#   pipwin install cairocffi
#
# Checked with Python 3.10
#

from pathlib import Path

from ezdxf import recover
from ezdxf.addons.drawing import matplotlib

import cairosvg, cv2, os, numpy as np
import glob

def convert_svg_to_png(filepath: Path):
    # Read svg and convert it to png.
    png_bytes = cairosvg.svg2png( url=str(filepath) )
    png = cv2.imdecode( np.frombuffer(png_bytes, dtype=np.uint8), cv2.IMREAD_UNCHANGED )

    # Make mask of where the transparent bits are.
    trans_mask = png[:,:,3] == 0

    # Replace areas of transparency with white and not transparent.
    png[trans_mask] = [255, 255, 255, 255]

    # New image without alpha channel.
    return cv2.cvtColor(png, cv2.COLOR_BGRA2BGR)


def compare_png(cur_png, ref_png, diff_filename: str):
    diff_ok = True

    # Compare images size.
    assert cur_png.shape[0] == ref_png.shape[0]
    assert cur_png.shape[1] == ref_png.shape[1]

    # Get difference.
    lost_lines = cv2.subtract(ref_png, cur_png)
    new_lines  = cv2.subtract(cur_png, ref_png)

    # Check results.
    empty_color = [0, 0, 0]

    has_new_lines  = not np.all(new_lines == empty_color)
    lost_old_lines = not np.all(lost_lines == empty_color)

    # Save png image with difference.
    if has_new_lines or lost_old_lines:
        diff_ok = False

        diff_path = os.path.join( "D:/data/resold", ( diff_filename + '_diff.png' ) )

        alpha_new_lines  = np.sum(new_lines,  axis=-1) > 0
        alpha_lost_lines = np.sum(lost_lines, axis=-1) > 0

        new_lines[alpha_new_lines]   = [0, 0, 255]
        lost_lines[alpha_lost_lines] = [0, 255, 0]

        diff = new_lines + lost_lines

        cv2.imwrite(diff_path, diff)

    return diff_ok


def compare_svg(svg_path: Path, svg_path_ref: Path):
    assert svg_path.exists()
    assert svg_path_ref.exists()

    try:
        ref_png = convert_svg_to_png( svg_path_ref )
    except:
        print("Failed to convert SVG to PNG:")
        print(svg_path_ref)
        return False

    try:
        cur_png = convert_svg_to_png( svg_path )
    except:
        print("Failed to convert SVG to PNG:")
        print(svg_path)
        return False

    return compare_png(cur_png, ref_png, svg_path_ref.stem)


def convert_dxf_to_png(dxf_path: Path):
    assert dxf_path.exists()

    doc, auditor = recover.readfile( dxf_path )

    assert not auditor.has_errors

    png_path = os.path.join( "D:/data/resold", ( dxf_path.stem + '.png' ) )

    matplotlib.qsave(doc.modelspace(), png_path)

    assert Path(png_path).exists()

    png_content = cv2.imread(png_path, cv2.IMREAD_UNCHANGED)

    return cv2.cvtColor(png_content, cv2.COLOR_BGRA2BGR)


def compare_dxf(dxf_path: Path, dxf_path_ref: Path):
    assert dxf_path.exists()
    assert dxf_path_ref.exists()

    cur_png = convert_dxf_to_png(dxf_path)
    ref_png = convert_dxf_to_png(dxf_path_ref)

    return compare_png(cur_png, ref_png, dxf_path.stem)

def compare(oldpath: Path, newpath: Path, ext: str):
    exts = [ "svg", "dxf", "png" ]

    if not ext in exts:
        print( "Wrong extension passed - '" + ext + "'" )
        print( "Script supports - '" + ','.join(map(str, exts)) + "'" )
        return

    ff_old = glob.glob( oldpath + "/*." + ext )
    ff_new = glob.glob( newpath + "/*." + ext )

    print("Number of old files " + str(len(ff_old)) )
    print("Number of new files " + str(len(ff_new)) )

    if len(ff_old) != len(ff_new):
        print( "Error: passed folders should contain the same number of files with same names." )
        return

    for o, n in zip(ff_old, ff_new):
        isOk = True

        if ext == "svg":
            isOk = compare_svg( Path(n), Path(o) )

        if ext == "dxf":
            isOk = compare_dxf( Path(n), Path(o) )

        if ext == "png":
            isOk = compare_png( Path(n), Path(o), Path(o).stem )

        if ( not isOk ):
            print( "Difference is found:" )
            print( "\told: " + o )
            print( "\tnew: " + n )
