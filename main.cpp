#include "Urho3DAll.h"

using namespace Urho3D;

class MyApp : public Application
{
public:
    
    SharedPtr<Scene> scene;
    SharedPtr<Viewport> vp;
    SharedPtr<Camera> camera;
    SharedPtr<Node> cameraNode;
    SharedPtr<Node> brushNode;
    SharedPtr<StaticModel> brushModel;
    SharedPtr<Material> brushMaterial;

    SharedPtr<Node> previewBrushNode;
    SharedPtr<StaticModel> previewBrushModel;
    SharedPtr<Material> previewBrushMaterial;

    const float defaultBrushSize = 5.0;
    const float defaultBrushSizeMax = 10.0;
    const float defaultBrushSizeMin = 1.0;

    float brushSize; 
    
    SharedPtr<RenderPath> rp;
    bool goToClearRTT;
    int goToClearRTTState;

    Color brushColor;

    
    MyApp(Context* context) :
        Application(context)
    {

    }

    virtual void Setup()
    {
        // Called before engine initialization. engineParameters_ member variable can be modified here
        engineParameters_["WindowTitle"] = "Kazimir Malevich studio pro :)";
        engineParameters_["FullScreen"] = false;
        engineParameters_["Headless"] = false;
        engineParameters_["WindowWidth"] = 1280;
        engineParameters_["WindowHeight"] = 720;
        engineParameters_["RenderPath"] = "Bin/CoreData/RenderPaths/Forward.xml";
        engineParameters_["FrameLimiter"] = false;
        engineParameters_["FlushGPU"] = true;
    }

    virtual void Start()
    {
        

        scene = SharedPtr<Scene>(new Scene(context_));
        File sceneFile(context_, context_->GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/PaintScene.xml", FILE_READ);
        scene->LoadXML(sceneFile);

        cameraNode = scene->GetChild("Camera");
        camera = cameraNode->GetComponent<Camera>();
        if (camera)
        {
            ResourceCache* cache = GetSubsystem<ResourceCache>();
            camera->SetViewMask(1); // camera must see only first layer
            vp = SharedPtr<Viewport>(new Viewport(context_, scene, camera));
            vp->SetRenderPath(cache->GetResource<XMLFile>("CoreData/RenderPaths/ForwardPaint.xml"));
            rp = vp->GetRenderPath();
            Renderer* renderer = GetSubsystem<Renderer>();
            
            renderer->SetViewport(0, vp);
        }

        brushNode = scene->GetChild("Brush");
        brushModel = brushNode->GetComponent<StaticModel>();
        brushModel->SetViewMask(2); // hide painbrush by default at program startup
        brushColor = Color(1.0f,1.0f,1.0f,1.0f);
        brushMaterial = brushModel->GetMaterial();

        previewBrushNode = brushNode->GetChild("PreviewBrush");
        previewBrushModel = previewBrushNode->GetComponent<StaticModel>();
        previewBrushModel->SetViewMask(1); // it have opposite view mask to real paintbrush - brushModel, mask = 1;
        previewBrushMaterial = previewBrushModel->GetMaterial(); // we try to keep preview mat in actual settings as and main brushMaterial for user preview

        brushSize = defaultBrushSize;

        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(MyApp, HandleKeyDown));
        SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(MyApp, HandleMouseMove));
        SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(MyApp, HandleMouseButtonDown));
        SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(MyApp, HandleMouseButtonUp));
        SubscribeToEvent(E_MOUSEWHEEL, URHO3D_HANDLER(MyApp, HandleMouseWheel));


        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MyApp, HandleUpdate));


        Input* input = GetSubsystem<Input>();
        input->SetMouseVisible(true); // activate mouse cursor
        
    }

    Color GetRandomColor(float brithness = 0.5f) const
    {
        float r = Urho3D::Random(Random(1.0f) < 0.5f ? 0.0f : brithness, 1.0f);
        float g = Urho3D::Random(Random(1.0f) < 0.5f ? 0.0f : brithness, 1.0f);
        float b = Urho3D::Random(Random(1.0f) < 0.5f ? 0.0f : brithness, 1.0f);
        float a = Urho3D::Random(Random(1.0f) < 0.5f ? 0.0f : brithness, 1.0f);

        return Color(r,g,b,a);
    }

    virtual void Stop()
    {
        
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;

        if (goToClearRTT)
        {
        
            rp->SetEnabled("ClearCanvas", true); // change state for RenderPath command see (ForwardPaint.xml)
            goToClearRTT = false;
            goToClearRTTState = 1;
        }
        else 
        {
            if (goToClearRTTState == 1)
                rp->SetEnabled("ClearCanvas", false);
                goToClearRTTState = 0;
        }
    
    }

    void HandleKeyDown(StringHash eventType, VariantMap& eventData)
    {
        using namespace KeyDown;
        bool colorChange = false;

        int key = eventData[P_KEY].GetInt();
        if (key == KEY_ESC)
            engine_->Exit();
        if (key == KEY_DELETE | key == KEY_X)
            goToClearRTT = true; // for this, we still needed one frame, so we - set the flag for clearning and other work by clearing of RTT is in HandleUpdate() proc

        if (key == KEY_R) // just get random color for Brush and Preview brush
        {
            brushColor = GetRandomColor();
            colorChange = true;
        }

        if (key == KEY_1) // just get red
        {
            brushColor = Color(1.0f,0.0f,0.0f);
            colorChange = true;
        }

        if (key == KEY_2) // just get green
        {
            brushColor = Color(0.0f, 1.0f, 0.0f);
            colorChange = true;
        }

        if (key == KEY_3) // just get blue
        {
            brushColor = Color(0.0f, 0.0f, 1.0f);
            colorChange = true;
        }
        
        if (key == KEY_4) // just get white
        {
            brushColor = Color(1.0f, 1.0f, 1.0f);
            colorChange = true;
        }


        if (colorChange) 
        {
            brushMaterial->SetShaderParameter("MatDiffColor", Variant(brushColor));
            previewBrushMaterial->SetShaderParameter("MatDiffColor", Variant(brushColor));
        }
    }

    void HandleMouseMove(StringHash eventType, VariantMap& eventData)
    {
        using namespace MouseMove;

        int btn = eventData[P_BUTTONS].GetInt();

        int x = eventData[P_X].GetInt();
        int y = eventData[P_Y].GetInt();
        
        float z = (brushSize > defaultBrushSizeMax) ? defaultBrushSizeMax : ((defaultBrushSizeMin > brushSize) ? defaultBrushSizeMin : brushSize);
        
        Vector3 projection = vp->ScreenToWorldPoint(x, y, z);

        brushNode->SetWorldPosition(projection);
        
        ActualizeBrushState(btn , -1);

    }
    
    void HandleMouseButtonDown(StringHash eventType, VariantMap& eventData) 
    {
        using namespace MouseButtonDown;

        int btn = eventData[P_BUTTON].GetInt();

        ActualizeBrushState(btn, 1);

        
    }

    void HandleMouseButtonUp(StringHash eventType, VariantMap& eventData) 
    {
        using namespace MouseButtonUp;

        int btn = eventData[P_BUTTON].GetInt();

        ActualizeBrushState(btn, 0);
             
    }

    void HandleMouseWheel(StringHash eventType, VariantMap& eventData) 
    {
        using namespace MouseWheel;

        int wheel = eventData[P_WHEEL].GetInt();
        
        if (wheel > 0)
        {
            if (brushSize < defaultBrushSizeMax)
                brushSize += 0.2;
            else
                brushSize = defaultBrushSizeMax;
        }
        else if (wheel < 0)
        {
            if (brushSize > defaultBrushSizeMin)
                brushSize -= 0.2;
            else
                brushSize = defaultBrushSizeMin;
        }
    }
      
    void ActualizeBrushState(int btn, int down = -1) 
    {
        static bool updateBrushColorAfterEraseOp;
        bool drawing = false;

        if (btn == MOUSEB_LEFT)
        {
            if (down == 1) // if drawing start with btn down
            {
                brushModel->SetViewMask(1); // if mouse left btn are pressed - show brush (render to RTT)
                rp->SetEnabled("Drawning", true); // change state for RenderPath command see (ForwardPaint.xml
                previewBrushModel->SetViewMask(0); // hide preview brush at all white user doing paint process on RTT
            }
            else if (down == 0) // if drawing end with btn release
            {
                brushModel->SetViewMask(0); // otherwise - hide brush for RTT
                rp->SetEnabled("Drawning", false); // change state for RenderPath command see (ForwardPaint.xm
                previewBrushModel->SetViewMask(1); // return preview brush, show it for user   
            }
        }
        else if (btn == MOUSEB_RIGHT)
        {
            if (down == 1)
            {
                brushMaterial->SetShaderParameter("MatDiffColor", Variant(Color::BLACK));
                previewBrushMaterial->SetShaderParameter("MatDiffColor", Variant(Color::BLACK));
                updateBrushColorAfterEraseOp = true;
                
                brushModel->SetViewMask(1); // if mouse left btn are pressed - show brush (render to RTT)
                rp->SetEnabled("Drawning", true); // change state for RenderPath command see (ForwardPaint.xml
                previewBrushModel->SetViewMask(0); // hide preview brush at all white user doing paint process on RTT

            }
            else if (down == 0)
            {
                if (updateBrushColorAfterEraseOp) // return old color
                {
                    brushMaterial->SetShaderParameter("MatDiffColor", Variant(brushColor));
                    previewBrushMaterial->SetShaderParameter("MatDiffColor", Variant(brushColor));
                    updateBrushColorAfterEraseOp = false;
                }

                brushModel->SetViewMask(0); // otherwise - hide brush for RTT
                rp->SetEnabled("Drawning", false); // change state for RenderPath command see (ForwardPaint.xm
                previewBrushModel->SetViewMask(1); // return preview brush, show it for user               
            }

            
        }
    }


    

};

URHO3D_DEFINE_APPLICATION_MAIN(MyApp)
