#include "datasetInfo.h"
#include <GL/glew.h>

DatasetInfo::DatasetInfo()
{
	m_type = not_initialized;
	m_length = 0;
	m_bands = 0;
	m_frames = 0;
	m_rows = 0;
	m_columns = 0;
	m_repn = wxT("");
	m_xVoxel = 0.0;
	m_yVoxel = 0.0;
	m_zVoxel = 0.0;
	is_loaded = false;
	m_highest_value = 1.0;
	m_threshold = 0.10;
	m_show = true;
}

DatasetInfo::~DatasetInfo()
{
	switch (m_type)
		{
		case Head_byte:
			delete m_byteDataset;
			break;
		case Overlay:
			delete m_floatDataset;
			break;
		case RGB:
			delete m_rgbDataset;
			break;
		default:
			break;
		}
}

bool DatasetInfo::load(wxString filename)
{
	// check file extension
	wxString ext = filename.substr(filename.Length()-3,3);
	if (ext != wxT("hea")) return false;
	m_name = filename.AfterLast('/');
	//m_name = m_name.BeforeLast('.');
	// read header file
	wxTextFile headerFile;
	bool flag = false;
	if (headerFile.Open(filename))
	{
		size_t i;
		wxString sLine;
		wxString sValue;
		wxString sLabel;
		long lTmpValue;
		for (i = 3 ; i < headerFile.GetLineCount() ; ++i)
		{
			sLine = headerFile.GetLine(i);
			sLabel = sLine.BeforeLast(' ');
			sValue = sLine.AfterLast(' ');
			sLabel.Trim(false);
			sLabel.Trim();
			if (sLabel.Contains(wxT("length:"))) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				m_length = (int)lTmpValue;
			}
			if (sLabel == wxT("nbands:")) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				m_bands = (int)lTmpValue;
			}
			if (sLabel == wxT("nframes:")) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				m_frames = (int)lTmpValue;
			}
			if (sLabel == wxT("nrows:")) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				m_rows = (int)lTmpValue;
			}
			if (sLabel == wxT("ncolumns:")) 
			{
				flag = sValue.ToLong(&lTmpValue, 10);
				m_columns = (int)lTmpValue;
			}
			if (sLabel == wxT("repn:"))
			//if (sLabel.Contains(wxT("repn:"))) 
			{
				m_repn = sValue;
			}
			if (sLabel.Contains(wxT("voxel:"))) 
			{
				wxString sNumber;
				sValue = sLine.AfterLast(':');
				sValue = sValue.BeforeLast('\"');
				sNumber = sValue.AfterLast(' ');
				flag = sNumber.ToDouble(&m_zVoxel); 
				sValue = sValue.BeforeLast(' ');
				sNumber = sValue.AfterLast(' ');
				flag = sNumber.ToDouble(&m_yVoxel);
				sValue = sValue.BeforeLast(' ');
				sNumber = sValue.AfterLast('\"');
				flag = sNumber.ToDouble(&m_xVoxel);
			}
		}
	}
	headerFile.Close();
	
	if (m_repn.Cmp(wxT("ubyte")) == 0)
	{
		if (m_bands / m_frames == 1)
			m_type = Head_byte;
		else if (m_bands / m_frames == 3)
			m_type = RGB;
		else m_type = ERROR;
	}
	else if (m_repn.Cmp(wxT("short")) == 0) m_type = Head_short;
	else if (m_repn.Cmp(wxT("float")) == 0) m_type = Overlay;
	else m_type = ERROR;
	
	is_loaded = flag;
	return flag;
}

void DatasetInfo::generateTexture()
{
	switch (m_type)
	{
	case Head_byte:
		glTexImage3D(GL_TEXTURE_3D, 
			0, 
			GL_RGBA, 
			m_columns, 
			m_rows,
			m_frames,
			0, 
			GL_LUMINANCE, 
			GL_UNSIGNED_BYTE,
			m_byteDataset);
		break;
	case Overlay:
		glTexImage3D(GL_TEXTURE_3D, 
			0, 
			GL_RGB, 
			m_columns, 
			m_rows,
			m_frames,			
			0, 
			GL_LUMINANCE, 
			GL_FLOAT,
			m_floatDataset);
		break;
	case RGB:
		glTexImage3D(GL_TEXTURE_3D, 
			0, 
			GL_RGBA, 
			m_columns, 
			m_rows,
			m_frames,
			0, 
			GL_RGB, 
			GL_UNSIGNED_BYTE,
			m_rgbDataset);
		break;
	default:
		break;
	}
}


wxString DatasetInfo::getInfoString()
{
	if (!is_loaded) return wxT("not loaded");
	wxString infoString1, infoString2, infoString3;
	infoString1.Empty();
	infoString2.Empty();
	infoString3.Empty();
	infoString1 = wxString::Format(wxT("Length: %d\nBands: %d\nFrames: %d\nRows: %d\nColumns: %d\nRepn: "), 
			this->m_length, this->m_bands, this->m_frames, this->m_rows, this->m_columns) + this->m_repn;
	infoString2 = wxString::Format(wxT("\nx Voxel: %.2f\ny Voxel: %.2f\nz Voxel: %.2f"), this->m_xVoxel, this->m_yVoxel, this->m_zVoxel);
	return m_name + wxT(":\n") + infoString1;
}
