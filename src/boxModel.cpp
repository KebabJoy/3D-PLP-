class BoxModel
{
public:
    int Index { get; set; }
    int NumberBoxs { get; set; }
    float Volumn { get; set; }
    float Weight { get; set; }

    private BoxModel(){

    }

    public BoxModel(int index){
        this.Index = index;
    }

    public BoxModel Copy(){
        return new BoxModel
        {
            Index = this.Index,
            NumberBoxs = this.NumberBoxs,
            Volumn = this.Volumn,
            Weight = this.Weight
        };
    }
}
