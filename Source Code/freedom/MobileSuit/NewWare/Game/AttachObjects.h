
// ****************************************************************************************
//
//     Project : Dragonica Optimize and Refactoring
//   Copyright : Copyright (C) 2009 - 2010 Barunson Interactive, Inc
//        Name : AttachObjects.h
// Description : .
//      Author : Jae-Ryoung, Lee
//
// Remarks :
//   * AttachSlot m_kAttachSlot/m_kAttachSlot_NoZTest 대체 모듈임.
//   * 대체 모듈 적용시 PgAVObjectAlphaProcessManager를 ProcessManager를 상속 받게 바꾸어야 함!
//     (참고: AttachParticleObjects::RemoveProcess()함수 내부의 객체 타입-케스팅 바꾸기)
// 
// Revisions :
//  10/01/14 LeeJR First Created
//

#ifndef _GAME_ATTACHOBJECTS_H__
#define _GAME_ATTACHOBJECTS_H__


namespace NewWare
{

namespace Game
{

class ProcessManager;


class AttachObjects
{
public:
    enum
    {
        DRAW_NORMALS  = (1 << 0), 
        DRAW_NOZTESTS = (1 << 1), 
        DRAW_ALL = DRAW_NORMALS | DRAW_NOZTESTS
    };


protected:
    typedef int Key;
    typedef std::map<Key, NiAVObjectPtr>   NiAVObjectList;
    typedef NiAVObjectList::iterator       NiAVObjectListIter;
    typedef NiAVObjectList::const_iterator NiAVObjectListCIter;
    typedef NiAVObjectList::value_type     NiAVObjectListValueType;


public:
    AttachObjects();
    virtual ~AttachObjects();


    void OnUpdate( float fAccumTime, float fFrameTime, ProcessManager* pkProcessManager )
    {
        UpdateLifetime( fAccumTime, fFrameTime, pkProcessManager );
        UpdateTransform( fAccumTime, fFrameTime );
    }

    void Draw( NiCamera const* pkCamera, NiCullingProcess& kCuller, NiVisibleArray& kVisibleSet, DWORD dwMode )
    {
        if ( dwMode & DRAW_NORMALS )
            DrawList( m_mNormals,  pkCamera, kCuller, kVisibleSet );
        if ( dwMode & DRAW_NOZTESTS )
            DrawList( m_mNoZTests, pkCamera, kCuller, kVisibleSet );
    }

    virtual bool Add( Key key, NiAVObject* pkAVObject ) = 0;
    virtual bool Add( Key key, NiAVObject* pkAVObject, NiAVObject* pkAttachTarget ) = 0;
    bool Remove( Key key, ProcessManager* pkProcessManager )
    {
        if ( RemoveFrom(m_mNormals,  key, pkProcessManager) || 
             RemoveFrom(m_mNoZTests, key, pkProcessManager) )
            return true;
        return false;
    }
    void RemoveAll()
    {
        _RemoveAll( m_mNormals );
        _RemoveAll( m_mNoZTests );
    }

    NiAVObject* GetNodeByKeyName( Key key, char const* name ) const
    {
        NiAVObject* pkAVObject = _GetNodeByKeyName( m_mNormals, key, name );
        if ( pkAVObject )
            return pkAVObject;
        pkAVObject = _GetNodeByKeyName( m_mNoZTests, key, name );
        if ( pkAVObject )
            return pkAVObject;
        return NULL;
    }
    NiAVObject* GetNodeByExtraValue( std::string const& strValue, char const* extraDataName ) const
    {
        NiAVObject* pkAVObject = _GetNodeByExtraValue( m_mNormals, strValue, extraDataName );
        if ( pkAVObject )
            return pkAVObject;
        pkAVObject = _GetNodeByExtraValue( m_mNoZTests, strValue, extraDataName );
        if ( pkAVObject )
            return pkAVObject;
        return NULL;
    }
    bool GetWorldPositionByKeyName( NiPoint3& kPos, Key key, char const* name ) const
    {
        if ( _GetWorldPositionByKeyName(kPos, m_mNormals,  key, name) || 
             _GetWorldPositionByKeyName(kPos, m_mNoZTests, key, name) )
            return true;
        return false;
    }
    bool SetAlphaGroup( Key key, int iGroupNumber )
    {
        if ( _SetAlphaGroup(m_mNormals,  key, iGroupNumber) || 
             _SetAlphaGroup(m_mNoZTests, key, iGroupNumber) )
            return true;
        return false;
    }


protected:
    virtual void UpdateLifetime( float fAccumTime, float fFrameTime, ProcessManager* pkProcessManager ) { /* Hook method */ };
    virtual void UpdateTransform( float fAccumTime, float fFrameTime ) { /* Hook method */ };

    virtual void RemoveProcess( NiAVObject* pkAVObject, ProcessManager* pkProcessManager ) { /* Hook method */ };
    virtual void Delete( NiAVObject* pkAVObject );


protected:
    void DrawList( NiAVObjectList& mList, NiCamera const* pkCamera, 
                   NiCullingProcess& kCuller, NiVisibleArray& kVisibleSet );

    bool RemoveFrom( NiAVObjectList& mList, Key key, ProcessManager* pkProcessManager );
    void _RemoveAll( NiAVObjectList& mList );

    NiAVObject* _GetNodeByKeyName( NiAVObjectList const& mList, Key key, char const* name ) const;
    NiAVObject* _GetNodeByExtraValue( NiAVObjectList const& mList, 
                                      std::string const& strValue, char const* extraDataName ) const;
    bool _GetWorldPositionByKeyName( NiPoint3& kPos, const NiAVObjectList& mList, Key key, char const* name ) const;
    bool _SetAlphaGroup( NiAVObjectList const& mList, Key key, int iGroupNumber );


protected:
    NiAVObjectList m_mNormals;
    NiAVObjectList m_mNoZTests;
};


//==================================================================================================


class AttachParticleObjects : public AttachObjects
{
public:
    virtual bool Add( Key key, NiAVObject* pkAVObject );
    virtual bool Add( Key key, NiAVObject* pkAVObject, NiAVObject* pkAttachTarget );

    bool PlayGenerate( Key key, bool bGenerate, bool bRemove = true )
    {
        if ( _PlayGenerate(m_mNormals,  key, bGenerate, bRemove) || 
             _PlayGenerate(m_mNoZTests, key, bGenerate, bRemove) )
            return true;
        return false;
    }
    void FadeOut( char const* extraDataName )
    {
        _FadeOut( m_mNormals,  extraDataName );
        _FadeOut( m_mNoZTests, extraDataName );
    }
    bool DetachAlphaProperty( Key key, NiAlphaProperty* pkAlphaProp )
    {
        if ( _DetachAlphaProperty(m_mNormals,  key, pkAlphaProp) || 
             _DetachAlphaProperty(m_mNoZTests, key, pkAlphaProp) )
            return true;
        return false;
    }
    void SetDefaultMaterialNeedsUpdateFlag( bool bFlag )
    {
        _SetDefaultMaterialNeedsUpdateFlag( m_mNormals,  bFlag );
        _SetDefaultMaterialNeedsUpdateFlag( m_mNoZTests, bFlag );
    }


protected:
    virtual void UpdateLifetime( float fAccumTime, float fFrameTime, ProcessManager* pkProcessManager )
    {
        _UpdateLifetime( m_mNormals,  fAccumTime, fFrameTime, pkProcessManager );
        _UpdateLifetime( m_mNoZTests, fAccumTime, fFrameTime, pkProcessManager );
    }
    virtual void UpdateTransform( float fAccumTime, float fFrameTime )
    {
        _UpdateTransform( m_mNormals,  fAccumTime, fFrameTime );
        _UpdateTransform( m_mNoZTests, fAccumTime, fFrameTime );
    }

    virtual void RemoveProcess( NiAVObject* pkAVObject, ProcessManager* pkProcessManager );


private:
    bool _PlayGenerate( NiAVObjectList& mList, Key key, bool bGenerate, bool bRemove );
    void _FadeOut( NiAVObjectList const& mList, char const* extraDataName );
    bool _DetachAlphaProperty( NiAVObjectList const& mList, Key key, NiAlphaProperty* pkAlphaProp );
    void _SetDefaultMaterialNeedsUpdateFlag( NiAVObjectList const& mList, bool bFlag );

    void _UpdateLifetime( NiAVObjectList& mList, float fAccumTime, float fFrameTime, ProcessManager* pkProcessManager );
    void _UpdateTransform( NiAVObjectList& mList, float fAccumTime, float fFrameTime );
};


} //namespace Game

} //namespace NewWare


#endif //_GAME_ATTACHOBJECTS_H__
