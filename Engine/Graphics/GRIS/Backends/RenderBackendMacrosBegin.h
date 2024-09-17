#if GRIS_RENDERBACKEND_IMP == GRIS_RENDERBACKEND_IMP_ABSTRACT
#define GRIS_RENDERBACKEND_CONSTRUCTOR( NAME ) NAME() = default;
#define GRIS_RENDERBACKEND_DESTRUCTOR( NAME ) virtual ~NAME() = default;
#define GRIS_RENDERBACKEND_CLASS_NAME( NAME ) class NAME
#define GRIS_RENDERBACKEND_FUNC( FUNC_SIG, RETURN_EXPR )                  \
   virtual FUNC_SIG                                                       \
   {                                                                      \
      CYD_ASSERT( !"Function used and unimplemented in render backend" ); \
      RETURN_EXPR;                                                        \
   }
#define GRIS_RENDERBACKEND_PIMPL( NAME )
#endif

#if GRIS_RENDERBACKEND_IMP == GRIS_RENDERBACKEND_IMP_CONCRETE
#define GRIS_RENDERBACKEND_CONSTRUCTOR( NAME ) NAME( const Window& window );
#define GRIS_RENDERBACKEND_DESTRUCTOR( NAME ) ~NAME();
#define GRIS_RENDERBACKEND_CLASS_NAME( NAME ) class NAME final : public RenderBackend
#define GRIS_RENDERBACKEND_FUNC( FUNC_SIG, RETURN_EXPR ) FUNC_SIG override;
#define GRIS_RENDERBACKEND_PIMPL( NAME ) \
  private:                               \
   NAME##Imp* _imp;
#endif