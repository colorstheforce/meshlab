/****************************************************************************
* MeshLab                                                           o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004-2020                                           \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/


#include <QString>
#include <QtGlobal>
#include <QFileInfo>

#include "mesh_model.h"
#include "mesh_document.h"

#include <wrap/gl/math.h>

#include <QDir>
#include <utility>

using namespace vcg;

void MeshModel::Clear()
{
	setMeshModified(false);
    // These data are always active on the mesh
    currentDataMask = MM_NONE;
    currentDataMask |= MM_VERTCOORD | MM_VERTNORMAL | MM_VERTFLAG ;
    currentDataMask |= MM_FACEVERT  | MM_FACENORMAL | MM_FACEFLAG ;

    visible=true;
    cm.Tr.SetIdentity();
    cm.sfn=0;
    cm.svn=0;
}

void MeshModel::UpdateBoxAndNormals()
{
    tri::UpdateBounding<CMeshO>::Box(cm);
    if(cm.fn>0) {
        tri::UpdateNormal<CMeshO>::PerFaceNormalized(cm);
        tri::UpdateNormal<CMeshO>::PerVertexAngleWeighted(cm);
    }
}

MeshModel::MeshModel(MeshDocument *_parent, const QString& fullFileName, const QString& labelName)
{
    /*glw.m = &(cm);*/
    Clear();
    parent=_parent;
    _id=parent->newMeshId();
    if(!fullFileName.isEmpty())   this->fullPathFileName=fullFileName;
    if(!labelName.isEmpty())     this->_label=labelName;
}

MeshModel::MeshModel(MeshModel* cp)
{
	if (cp == NULL)
		return;
	parent = cp->parent;
	if (parent != NULL)
		_id = parent->newMeshId();
	cm.Tr = cp->cm.Tr;
	cm.sfn = cp->cm.sfn;
	cm.svn = cp->cm.svn;
	visible = cp->visible;
	updateDataMask(cp->currentDataMask);
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(cm, cp->cm);
}

QString MeshModel::relativePathName() const
{
    QDir documentDir (documentPathName());
    QString relPath=documentDir.relativeFilePath(this->fullPathFileName);

    if(relPath.size()>1 && relPath[0]=='.' &&  relPath[1]=='.')
        qDebug("Error we have a mesh that is not in the same folder of the project: %s ", qUtf8Printable(relPath));

    return relPath;
}

QString MeshModel::documentPathName() const
{
    return parent->pathName();
}

int MeshModel::io2mm(int single_iobit)
{
    switch(single_iobit)
    {
    case tri::io::Mask::IOM_NONE					: return  MM_NONE;
    case tri::io::Mask::IOM_VERTCOORD		: return  MM_VERTCOORD;
    case tri::io::Mask::IOM_VERTCOLOR		: return  MM_VERTCOLOR;
    case tri::io::Mask::IOM_VERTFLAGS		: return  MM_VERTFLAG;
    case tri::io::Mask::IOM_VERTQUALITY	: return  MM_VERTQUALITY;
    case tri::io::Mask::IOM_VERTNORMAL		: return  MM_VERTNORMAL;
    case tri::io::Mask::IOM_VERTTEXCOORD : return  MM_VERTTEXCOORD;
    case tri::io::Mask::IOM_VERTRADIUS		: return  MM_VERTRADIUS;

    case tri::io::Mask::IOM_FACEINDEX   		: return  MM_FACEVERT  ;
    case tri::io::Mask::IOM_FACEFLAGS   		: return  MM_FACEFLAG  ;
    case tri::io::Mask::IOM_FACECOLOR   		: return  MM_FACECOLOR  ;
    case tri::io::Mask::IOM_FACEQUALITY 		: return  MM_FACEQUALITY;
    case tri::io::Mask::IOM_FACENORMAL  		: return  MM_FACENORMAL ;

    case tri::io::Mask::IOM_WEDGTEXCOORD 		: return  MM_WEDGTEXCOORD;
    case tri::io::Mask::IOM_WEDGCOLOR				: return  MM_WEDGCOLOR;
    case tri::io::Mask::IOM_WEDGNORMAL   		: return  MM_WEDGNORMAL  ;

    case tri::io::Mask::IOM_BITPOLYGONAL   	: return  MM_POLYGONAL  ;

    default:
        assert(0);
        return MM_NONE;  // FIXME: Returning this is not the best solution (!)
        break;
    } ;
}








void MeshModelState::create(int _mask, MeshModel* _m)
{
    m=_m;
    changeMask=_mask;
    if(changeMask & MeshModel::MM_VERTCOLOR)
    {
        vertColor.resize(m->cm.vert.size());
        std::vector<Color4b>::iterator ci;
        CMeshO::VertexIterator vi;
        for(vi = m->cm.vert.begin(), ci = vertColor.begin(); vi != m->cm.vert.end(); ++vi, ++ci)
            if(!(*vi).IsD()) (*ci)=(*vi).C();
    }

    if(changeMask & MeshModel::MM_VERTQUALITY)
    {
        vertQuality.resize(m->cm.vert.size());
        std::vector<float>::iterator qi;
        CMeshO::VertexIterator vi;
        for(vi = m->cm.vert.begin(), qi = vertQuality.begin(); vi != m->cm.vert.end(); ++vi, ++qi)
            if(!(*vi).IsD()) (*qi)=(*vi).Q();
    }

    if(changeMask & MeshModel::MM_VERTCOORD)
    {
        vertCoord.resize(m->cm.vert.size());
        std::vector<Point3m>::iterator ci;
        CMeshO::VertexIterator vi;
        for(vi = m->cm.vert.begin(), ci = vertCoord.begin(); vi != m->cm.vert.end(); ++vi, ++ci)
            if(!(*vi).IsD()) (*ci)=(*vi).P();
    }

    if(changeMask & MeshModel::MM_VERTNORMAL)
    {
        vertNormal.resize(m->cm.vert.size());
        std::vector<Point3m>::iterator ci;
        CMeshO::VertexIterator vi;
        for(vi = m->cm.vert.begin(), ci = vertNormal.begin(); vi != m->cm.vert.end(); ++vi, ++ci)
            if(!(*vi).IsD()) (*ci)=(*vi).N();
    }

    if(changeMask & MeshModel::MM_FACENORMAL)
    {
        faceNormal.resize(m->cm.face.size());
        std::vector<Point3m>::iterator ci;
        CMeshO::FaceIterator fi;
        for(fi = m->cm.face.begin(), ci = faceNormal.begin(); fi != m->cm.face.end(); ++fi, ++ci)
            if(!(*fi).IsD()) (*ci) = (*fi).N();
    }

    if(changeMask & MeshModel::MM_FACECOLOR)
    {
        m->updateDataMask(MeshModel::MM_FACECOLOR);
        faceColor.resize(m->cm.face.size());
        std::vector<Color4b>::iterator ci;
        CMeshO::FaceIterator fi;
        for(fi = m->cm.face.begin(), ci = faceColor.begin(); fi != m->cm.face.end(); ++fi, ++ci)
            if(!(*fi).IsD()) (*ci) = (*fi).C();
    }

    if(changeMask & MeshModel::MM_FACEFLAGSELECT)
    {
        faceSelection.resize(m->cm.face.size());
        std::vector<bool>::iterator ci;
        CMeshO::FaceIterator fi;
        for(fi = m->cm.face.begin(), ci = faceSelection.begin(); fi != m->cm.face.end(); ++fi, ++ci)
            if(!(*fi).IsD()) (*ci) = (*fi).IsS();
    }

    if(changeMask & MeshModel::MM_VERTFLAGSELECT)
    {
        vertSelection.resize(m->cm.vert.size());
        std::vector<bool>::iterator ci;
        CMeshO::VertexIterator vi;
        for(vi = m->cm.vert.begin(), ci = vertSelection.begin(); vi != m->cm.vert.end(); ++vi, ++ci)
            if(!(*vi).IsD()) (*ci) = (*vi).IsS();
    }

    if(changeMask & MeshModel::MM_TRANSFMATRIX)
        Tr = m->cm.Tr;
    if(changeMask & MeshModel::MM_CAMERA)
        this->shot = m->cm.shot;
}

bool MeshModelState::apply(MeshModel *_m)
{
    if(_m != m)
        return false;
    if(changeMask & MeshModel::MM_VERTCOLOR)
    {
        if(vertColor.size() != m->cm.vert.size()) return false;
        std::vector<Color4b>::iterator ci;
        CMeshO::VertexIterator vi;
        for(vi = m->cm.vert.begin(), ci = vertColor.begin(); vi != m->cm.vert.end(); ++vi, ++ci)
            if(!(*vi).IsD()) (*vi).C()=(*ci);
    }
    if(changeMask & MeshModel::MM_FACECOLOR)
    {
        if(faceColor.size() != m->cm.face.size()) return false;
        std::vector<Color4b>::iterator ci;
        CMeshO::FaceIterator fi;
        for(fi = m->cm.face.begin(), ci = faceColor.begin(); fi != m->cm.face.end(); ++fi, ++ci)
            if(!(*fi).IsD()) (*fi).C()=(*ci);
    }
    if(changeMask & MeshModel::MM_VERTQUALITY)
    {
        if(vertQuality.size() != m->cm.vert.size()) return false;
        std::vector<float>::iterator qi;
        CMeshO::VertexIterator vi;
        for(vi = m->cm.vert.begin(), qi = vertQuality.begin(); vi != m->cm.vert.end(); ++vi, ++qi)
            if(!(*vi).IsD()) (*vi).Q()=(*qi);
    }

    if(changeMask & MeshModel::MM_VERTCOORD)
    {
        if(vertCoord.size() != m->cm.vert.size()) 
			return false;
        std::vector<Point3m>::iterator ci;
        CMeshO::VertexIterator vi;
        for(vi = m->cm.vert.begin(), ci = vertCoord.begin(); vi != m->cm.vert.end(); ++vi, ++ci)
            if(!(*vi).IsD()) 
				(*vi).P()=(*ci);
    }

    if(changeMask & MeshModel::MM_VERTNORMAL)
    {
        if(vertNormal.size() != m->cm.vert.size()) return false;
        std::vector<Point3m>::iterator ci;
        CMeshO::VertexIterator vi;
        for(vi = m->cm.vert.begin(), ci=vertNormal.begin(); vi != m->cm.vert.end(); ++vi, ++ci)
            if(!(*vi).IsD()) (*vi).N()=(*ci);
    }

    if(changeMask & MeshModel::MM_FACENORMAL)
    {
        if(faceNormal.size() != m->cm.face.size()) return false;
        std::vector<Point3m>::iterator ci;
        CMeshO::FaceIterator fi;
        for(fi = m->cm.face.begin(), ci=faceNormal.begin(); fi != m->cm.face.end(); ++fi, ++ci)
            if(!(*fi).IsD()) (*fi).N()=(*ci);
    }

    if(changeMask & MeshModel::MM_FACEFLAGSELECT)
    {
        if(faceSelection.size() != m->cm.face.size()) return false;
        std::vector<bool>::iterator ci;
        CMeshO::FaceIterator fi;
        for(fi = m->cm.face.begin(), ci = faceSelection.begin(); fi != m->cm.face.end(); ++fi, ++ci)
        {
            if((*ci))
                (*fi).SetS();
            else
                (*fi).ClearS();
        }
    }

    if(changeMask & MeshModel::MM_VERTFLAGSELECT)
    {
        if(vertSelection.size() != m->cm.vert.size()) return false;
        std::vector<bool>::iterator ci;
        CMeshO::VertexIterator vi;
        for(vi = m->cm.vert.begin(), ci = vertSelection.begin(); vi != m->cm.vert.end(); ++vi, ++ci)
        {
            if((*ci))
                (*vi).SetS();
            else
                (*vi).ClearS();
        }
    }


    if(changeMask & MeshModel::MM_TRANSFMATRIX)
        m->cm.Tr=Tr;
    if(changeMask & MeshModel::MM_CAMERA)
        m->cm.shot = this->shot;

    return true;
}

/**** DATAMASK STUFF ****/

void MeshDocument::setVisible(int meshId, bool val)
{
    getMesh(meshId)->visible=val;
    emit meshSetChanged();
}

bool MeshModel::hasDataMask(const int maskToBeTested) const
{
    return ((currentDataMask & maskToBeTested)!= 0);
}
void MeshModel::updateDataMask(MeshModel *m)
{
    updateDataMask(m->currentDataMask);
}

void MeshModel::updateDataMask(int neededDataMask)
{
    if((neededDataMask & MM_FACEFACETOPO)!=0)
    {
        cm.face.EnableFFAdjacency();
        tri::UpdateTopology<CMeshO>::FaceFace(cm);
    }
    if((neededDataMask & MM_VERTFACETOPO)!=0)
    {
        cm.vert.EnableVFAdjacency();
        cm.face.EnableVFAdjacency();
        tri::UpdateTopology<CMeshO>::VertexFace(cm);
    }

    if((neededDataMask & MM_WEDGTEXCOORD)!=0)  
        cm.face.EnableWedgeTexCoord();
    if((neededDataMask & MM_FACECOLOR)!=0)     
        cm.face.EnableColor();
    if((neededDataMask & MM_FACEQUALITY)!=0)
        cm.face.EnableQuality();
    if((neededDataMask & MM_FACECURVDIR)!=0)   
        cm.face.EnableCurvatureDir();
    if((neededDataMask & MM_FACEMARK)!=0)	     
        cm.face.EnableMark();
    if((neededDataMask & MM_VERTMARK)!=0)      
        cm.vert.EnableMark();
    if((neededDataMask & MM_VERTCURV)!=0)      
        cm.vert.EnableCurvature();
    if((neededDataMask & MM_VERTCURVDIR)!=0)   
        cm.vert.EnableCurvatureDir();
    if((neededDataMask & MM_VERTRADIUS)!=0)    
        cm.vert.EnableRadius();
    if((neededDataMask & MM_VERTTEXCOORD)!=0)  
        cm.vert.EnableTexCoord();

    currentDataMask |= neededDataMask;
}

void MeshModel::clearDataMask(int unneededDataMask)
{
    if( ( (unneededDataMask & MM_VERTFACETOPO)!=0)	&& hasDataMask(MM_VERTFACETOPO)) {cm.face.DisableVFAdjacency();
    cm.vert.DisableVFAdjacency(); }
    if( ( (unneededDataMask & MM_FACEFACETOPO)!=0)	&& hasDataMask(MM_FACEFACETOPO))	cm.face.DisableFFAdjacency();

    if( ( (unneededDataMask & MM_WEDGTEXCOORD)!=0)	&& hasDataMask(MM_WEDGTEXCOORD)) 	cm.face.DisableWedgeTexCoord();
    if( ( (unneededDataMask & MM_FACECOLOR)!=0)			&& hasDataMask(MM_FACECOLOR))			cm.face.DisableColor();
    if( ( (unneededDataMask & MM_FACEQUALITY)!=0)		&& hasDataMask(MM_FACEQUALITY))		cm.face.DisableQuality();
    if( ( (unneededDataMask & MM_FACEMARK)!=0)			&& hasDataMask(MM_FACEMARK))			cm.face.DisableMark();
    if( ( (unneededDataMask & MM_VERTMARK)!=0)			&& hasDataMask(MM_VERTMARK))			cm.vert.DisableMark();
    if( ( (unneededDataMask & MM_VERTCURV)!=0)			&& hasDataMask(MM_VERTCURV))			cm.vert.DisableCurvature();
    if( ( (unneededDataMask & MM_VERTCURVDIR)!=0)		&& hasDataMask(MM_VERTCURVDIR))		cm.vert.DisableCurvatureDir();
    if( ( (unneededDataMask & MM_VERTRADIUS)!=0)		&& hasDataMask(MM_VERTRADIUS))		cm.vert.DisableRadius();
    if( ( (unneededDataMask & MM_VERTTEXCOORD)!=0)	&& hasDataMask(MM_VERTTEXCOORD))	cm.vert.DisableTexCoord();

    currentDataMask = currentDataMask & (~unneededDataMask);
}

void MeshModel::Enable(int openingFileMask)
{
    if( openingFileMask & tri::io::Mask::IOM_VERTTEXCOORD )
        updateDataMask(MM_VERTTEXCOORD);
    if( openingFileMask & tri::io::Mask::IOM_WEDGTEXCOORD )
        updateDataMask(MM_WEDGTEXCOORD);
    if( openingFileMask & tri::io::Mask::IOM_VERTCOLOR    )
        updateDataMask(MM_VERTCOLOR);
    if( openingFileMask & tri::io::Mask::IOM_FACECOLOR    )
        updateDataMask(MM_FACECOLOR);
    if( openingFileMask & tri::io::Mask::IOM_VERTRADIUS   ) updateDataMask(MM_VERTRADIUS);
    if( openingFileMask & tri::io::Mask::IOM_CAMERA       ) updateDataMask(MM_CAMERA);
    if( openingFileMask & tri::io::Mask::IOM_VERTQUALITY  ) updateDataMask(MM_VERTQUALITY);
    if( openingFileMask & tri::io::Mask::IOM_FACEQUALITY  ) updateDataMask(MM_FACEQUALITY);
    if( openingFileMask & tri::io::Mask::IOM_BITPOLYGONAL ) updateDataMask(MM_POLYGONAL);
}

bool MeshModel::meshModified() const
{
	return modified;
}

void MeshModel::setMeshModified(bool b)
{
	modified = b;
}

int MeshModel::dataMask() const
{
    return currentDataMask;
}
